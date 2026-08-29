#pragma once

#include "vent_panel_protocol.h"

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure the panel-bus UART
 *
 * @note  Must be called at least once before starting the reader task
 */
esp_err_t vent_panel_reader_init(void);

/** @brief Start the background task that reads and decodes panel-bus frames */
void vent_panel_reader_start_task(UBaseType_t priority);

/** @brief Thread-safe snapshot of the most recently decoded panel status */
vent_panel_state_t vent_panel_reader_get_state(void);

/** @brief True if a valid status frame has been seen recently (bus is alive) */
bool vent_panel_reader_is_online(void);

#ifdef __cplusplus
}
#endif
