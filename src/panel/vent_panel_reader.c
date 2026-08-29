#include "vent_panel_reader.h"

#include "../drivers/vent_uart_driver.h"
#include "../project_config.h"
#include "../utilities/log_utils.h"

#include <esp_timer.h>
#include <freertos/semphr.h>
#include <string.h>

#define VENT_PANEL_TAG          "vent_panel_reader"

#define PANEL_CHUNK_MAX_LEN     32
#define PANEL_ONLINE_TIMEOUT_US (3000 * 1000)

static SemaphoreHandle_t state_mutex;
static QueueHandle_t uart_event_queue = NULL;
static vent_panel_state_t last_state;
static volatile int64_t last_frame_us = 0;

/** @brief Publish a freshly decoded state and mark the bus as alive */
static void apply_decoded_state(const vent_panel_state_t *decoded);

/** @brief The main task that reads bytes from the panel UART, groups them into frames, and decodes status frames */
static void vent_panel_reader_task(void *arg);

void vent_panel_reader_start_task(UBaseType_t priority) {
  xTaskCreate(vent_panel_reader_task, "vent_panel_reader", 4096, NULL, priority, NULL);
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

esp_err_t vent_panel_reader_init(void) {
  state_mutex = xSemaphoreCreateMutex();
  if (!state_mutex) return ESP_ERR_NO_MEM;

  memset(&last_state, 0, sizeof(last_state));
  last_state.temp_level = TEMP_LEVEL_UNKNOWN;

  return vent_uart_driver_init(PANEL_UART_PORT, UART_RX_PIN, UART_TX_PIN, PANEL_UART_BAUD_RATE, PANEL_UART_RX_BUF_SIZE, PANEL_UART_RX_IDLE_BYTE_TIMES, &uart_event_queue);
}

static void apply_decoded_state(const vent_panel_state_t *decoded) {
  xSemaphoreTake(state_mutex, portMAX_DELAY);
  last_state = *decoded;
  xSemaphoreGive(state_mutex);
  last_frame_us = esp_timer_get_time();
}

static void vent_panel_reader_task(void *arg) {
  (void)arg;
  uint8_t chunk[PANEL_CHUNK_MAX_LEN];
  uart_event_t event;

  LOGN(VENT_PANEL_TAG, "panel reader task started");

  for (;;) {
    if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY) != pdTRUE) continue;

    if (event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL) {
      LOGW(VENT_PANEL_TAG, "uart overflow (type %d), flushing", (int)event.type);
      uart_flush_input(PANEL_UART_PORT);
      xQueueReset(uart_event_queue);
      continue;
    }
    if (event.type != UART_DATA) continue;

    size_t size = event.size > sizeof(chunk) ? sizeof(chunk) : event.size;
    int byte_count = vent_uart_driver_read_bytes(PANEL_UART_PORT, chunk, size, PANEL_UART_READ_TIMEOUT_MS);
    if (byte_count <= 0) continue;

    // drop any remainder of an oversized burst so the next event starts frame-aligned
    if (event.size > sizeof(chunk)) uart_flush_input(PANEL_UART_PORT);

    vent_panel_state_t decoded;
    if (byte_count == VENT_PANEL_STATUS_FRAME_LEN && vent_panel_protocol_decode_status(chunk, (size_t)byte_count, &decoded)) {
      apply_decoded_state(&decoded);
    }
  }
}
