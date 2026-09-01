#include "vent_panel_reader.h"

#include "../drivers/vent_uart_driver.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/semphr.h>
#include <string.h>

static const char *const TAG = "vent_panel_reader";

#define PANEL_CHUNK_MAX_LEN     32
#define PANEL_ONLINE_TIMEOUT_US (3000 * 1000)
#define TRACE_REPEAT_INTERVAL   100

static SemaphoreHandle_t state_mutex;
static QueueHandle_t uart_event_queue = NULL;
static vent_panel_state_t last_state;
static volatile int64_t last_frame_us = 0;
static vent_panel_reader_config_t cfg;
static vent_panel_state_cb_t state_cb = NULL;
static void *state_cb_ctx = NULL;
static volatile bool trace_enabled = false;
static volatile uint32_t filter_reset_count = 0;

/** @brief Publish a freshly decoded state and mark the bus as alive */
static void apply_decoded_state(const vent_panel_state_t *decoded);

/** @brief The main task that reads bytes from the panel UART, groups them into frames, and decodes status frames */
static void vent_panel_reader_task(void *arg);

void vent_panel_reader_start_task(UBaseType_t priority) {
  xTaskCreate(vent_panel_reader_task, TAG, 4096, NULL, priority, NULL);
}

vent_panel_state_t vent_panel_reader_get_state(void) {
  vent_panel_state_t snapshot;
  xSemaphoreTake(state_mutex, portMAX_DELAY);
  snapshot = last_state;
  xSemaphoreGive(state_mutex);
  return snapshot;
}

bool vent_panel_reader_is_online(void) {
  return (esp_timer_get_time() - last_frame_us) < PANEL_ONLINE_TIMEOUT_US;
}

void vent_panel_reader_set_trace(bool enabled) {
  trace_enabled = enabled;
}

bool vent_panel_reader_get_trace(void) {
  return trace_enabled;
}

uint32_t vent_panel_reader_filter_reset_count(void) {
  return filter_reset_count;
}

esp_err_t vent_panel_reader_init(const vent_panel_reader_config_t *config) {
  if (!config) return ESP_ERR_INVALID_ARG;
  cfg = *config;

  state_mutex = xSemaphoreCreateMutex();
  if (!state_mutex) return ESP_ERR_NO_MEM;

  memset(&last_state, 0, sizeof(last_state));
  last_state.air_temp_level = AIR_TEMP_LEVEL_UNKNOWN;

  return vent_uart_driver_init(cfg.uart_port, cfg.rx_pin, cfg.tx_pin, cfg.baud_rate, cfg.rx_buffer_size, cfg.rx_idle_byte_times, &uart_event_queue);
}

void vent_panel_reader_set_state_callback(vent_panel_state_cb_t callback, void *ctx) {
  xSemaphoreTake(state_mutex, portMAX_DELAY);
  state_cb = callback;
  state_cb_ctx = ctx;
  xSemaphoreGive(state_mutex);
}

static void apply_decoded_state(const vent_panel_state_t *decoded) {
  xSemaphoreTake(state_mutex, portMAX_DELAY);
  bool changed = memcmp(&last_state, decoded, sizeof(last_state)) != 0;
  last_state = *decoded;
  vent_panel_state_cb_t callback = state_cb;
  void *ctx = state_cb_ctx;
  xSemaphoreGive(state_mutex);
  last_frame_us = esp_timer_get_time();

  // invoked outside the mutex so a consumer may call back into the reader
  if (changed && callback) callback(decoded, ctx);
}

static void vent_panel_reader_task(void *arg) {
  (void)arg;
  uint8_t chunk[PANEL_CHUNK_MAX_LEN];
  uart_event_t event;

  uint8_t trace_prev[PANEL_CHUNK_MAX_LEN];
  int trace_prev_len = 0;
  uint32_t trace_repeats = 0;
  bool trace_was_on = false;

  ESP_LOGI(TAG, "panel reader task started");

  for (;;) {
    if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY) != pdTRUE) continue;

    if (event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL) {
      ESP_LOGW(TAG, "uart overflow (type %d), flushing", (int)event.type);
      uart_flush_input(cfg.uart_port);
      xQueueReset(uart_event_queue);
      continue;
    }
    if (event.type != UART_DATA) continue;

    size_t size = event.size > sizeof(chunk) ? sizeof(chunk) : event.size;
    int byte_count = vent_uart_driver_read_bytes(cfg.uart_port, chunk, size, cfg.read_timeout_ms);
    if (byte_count <= 0) continue;

    // drop any remainder of an oversized burst so the next event starts frame-aligned
    if (event.size > sizeof(chunk)) uart_flush_input(cfg.uart_port);

    vent_panel_state_t decoded;
    bool decoded_ok = byte_count >= VENT_PANEL_STATUS_FRAME_LEN &&
                      vent_panel_protocol_decode_status(chunk, VENT_PANEL_STATUS_FRAME_LEN, &decoded);
    if (decoded_ok) apply_decoded_state(&decoded);

    uint16_t buttons = 0;
    if (byte_count >= VENT_PANEL_STATUS_FRAME_LEN + VENT_PANEL_BUTTON_FRAME_LEN && vent_panel_protocol_decode_button(chunk + VENT_PANEL_STATUS_FRAME_LEN, (size_t)(byte_count - VENT_PANEL_STATUS_FRAME_LEN), &buttons)) {
      if (buttons & VENT_PANEL_BTN_FILTER_OVERRIDE) filter_reset_count++;
      ESP_LOGD(TAG, "panel button 0x%04x", (unsigned)buttons);
    }

    bool trace_on = trace_enabled;
    if (trace_on != trace_was_on) {
      trace_prev_len = 0;
      trace_repeats = 0;
      trace_was_on = trace_on;
    }

    if (trace_on) {
      bool same = byte_count == trace_prev_len && memcmp(chunk, trace_prev, (size_t)byte_count) == 0;
      if (same) {
        trace_repeats++;
        if (trace_repeats % TRACE_REPEAT_INTERVAL == 0)
          ESP_LOGI(TAG, "(same frame seen %lu times so far)", (unsigned long)trace_repeats);
      } else {
        if (trace_repeats > 1) ESP_LOGI(TAG, "(last frame seen %lu times)", (unsigned long)trace_repeats);
        ESP_LOGI(TAG, "rx %d byte%s%s", byte_count, byte_count == 1 ? "" : "s", decoded_ok ? "" : " (undecoded)");
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, chunk, (uint16_t)byte_count, ESP_LOG_INFO);
        memcpy(trace_prev, chunk, (size_t)byte_count);
        trace_prev_len = byte_count;
        trace_repeats = 1;
      }
    }
  }
}
