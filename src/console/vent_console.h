#pragma once

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register the console commands and start the REPL task
 *
 * Runs on the USB Serial/JTAG console. The REPL owns its own task, so this
 * returns as soon as the console is running
 *
 * @param[in]   priority   priority of the REPL task
 */
esp_err_t vent_console_start(UBaseType_t priority);

#ifdef __cplusplus
}
#endif
