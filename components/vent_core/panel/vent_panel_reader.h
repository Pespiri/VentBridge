#pragma once

#include "vent_panel_protocol.h"

#include <driver/uart.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hal/gpio_types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Panel-bus wiring and timing, supplied by the host application */
typedef struct VENT_PANEL_READER_CONFIG {
  uart_port_t uart_port;
  gpio_num_t rx_pin;
  gpio_num_t tx_pin;
  uint32_t baud_rate;
  size_t rx_buffer_size;
  uint8_t rx_idle_byte_times; /* idle gap that ends a frame, in byte times */
  uint32_t read_timeout_ms;
} vent_panel_reader_config_t;

/**
 * @brief Called from the reader task whenever the decoded panel state changes
 *
 * @note Runs in the reader task context, not the caller's. Keep it short and
 *       do not block; hand off to a queue if the consumer needs to do real work.
 */
typedef void (*vent_panel_state_cb_t)(const vent_panel_state_t *state, void *ctx);

/**
 * @brief Configure the panel-bus UART
 *
 * @note  Must be called at least once before starting the reader task
 */
esp_err_t vent_panel_reader_init(const vent_panel_reader_config_t *config);

/** @brief Start the background task that reads and decodes panel-bus frames */
void vent_panel_reader_start_task(UBaseType_t priority);

/** @brief Register a state-change observer; pass NULL to clear */
void vent_panel_reader_set_state_callback(vent_panel_state_cb_t callback, void *ctx);

/** @brief Thread-safe snapshot of the most recently decoded panel status */
vent_panel_state_t vent_panel_reader_get_state(void);

/** @brief True if a valid status frame has been seen recently (bus is alive) */
bool vent_panel_reader_is_online(void);

/**
 * @brief Hex-dump every received chunk to the log, decodable or not
 *
 * Off by default. Undecodable chunks are only visible with this on, so it is the
 * way to see traffic the protocol decoder does not yet understand.
 */
void vent_panel_reader_set_trace(bool enabled);
bool vent_panel_reader_get_trace(void);

/**
 * @brief Number of filter-reset presses seen coming from the panel itself
 *
 * Monotonic, so a consumer can poll it and act on any increase without needing
 * to clear anything. Counts the button frame on the bus, which is authoritative:
 * it fires even when the filter alarm was not lit, unlike watching the alarm clear.
 */
uint32_t vent_panel_reader_filter_reset_count(void);

#ifdef __cplusplus
}
#endif
