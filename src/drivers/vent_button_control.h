#pragma once

#include "../panel/vent_panel_types.h"

#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hal/gpio_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VENT_BUTTON_ENUM {
  BUTTON_FAN_UP,
  BUTTON_FAN_DOWN,
  BUTTON_TEMP_UP,
  BUTTON_TEMP_DOWN,
  BUTTON_FILTER,
  BUTTON_FILTER_LONG, // held until the panel state changes, or a 10s timeout
} vent_button_enum_t;

/** @brief GPIOs wired to the panel's button contacts */
typedef struct VENT_BUTTON_PINS {
  gpio_num_t fan_up;
  gpio_num_t fan_down;
  gpio_num_t temp_up;
  gpio_num_t temp_down;
  gpio_num_t filter;
} vent_button_pins_t;

/** @brief Configure button GPIOs and the internal command queue */
esp_err_t vent_button_control_init(const vent_button_pins_t *pins);

/** @brief Start task executing queued button commands */
void vent_button_control_start_task(UBaseType_t priority);

/** @brief Queue a single button press (non-blocking) */
esp_err_t vent_button_control_press(vent_button_enum_t button);

/** @brief Queue fan to move to target */
esp_err_t vent_button_control_move_fan_to(vent_fan_level_enum_t target_level);

/** @brief Queue temperature to move to target */
esp_err_t vent_button_control_move_temp_to(vent_temp_level_enum_t target_level);

#ifdef __cplusplus
}
#endif
