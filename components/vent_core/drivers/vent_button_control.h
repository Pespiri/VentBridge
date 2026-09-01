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
  BUTTON_AIRFLOW_UP,
  BUTTON_AIRFLOW_DOWN,
  BUTTON_AIR_TEMP_UP,
  BUTTON_AIR_TEMP_DOWN,
  // The panel ignores a short filter press by design; only the long press acts,
  // clearing the filter replacement light. Held until the panel state changes,
  // or a 10s timeout.
  BUTTON_FILTER_LONG,
} vent_button_enum_t;

/** @brief GPIOs wired to the panel's button contacts */
typedef struct VENT_BUTTON_PINS {
  gpio_num_t airflow_up;
  gpio_num_t airflow_down;
  gpio_num_t air_temp_up;
  gpio_num_t air_temp_down;
  gpio_num_t filter;
} vent_button_pins_t;

/** @brief Configure button GPIOs and the internal command queue */
esp_err_t vent_button_control_init(const vent_button_pins_t *pins);

/** @brief Start task executing queued button commands */
void vent_button_control_start_task(UBaseType_t priority);

/** @brief Queue a single button press (non-blocking) */
esp_err_t vent_button_control_press(vent_button_enum_t button);

/** @brief Queue airflow to move to target */
esp_err_t vent_button_control_move_airflow_to(vent_airflow_level_enum_t target_level);

/** @brief Queue air temperature to move to target */
esp_err_t vent_button_control_move_air_temp_to(vent_air_temp_level_enum_t target_level);

/**
 * @brief Level a queued or in-progress move is working toward
 *
 * A move is executed as repeated single-step presses, so the panel reports every
 * level in between. Consumers should report this target instead while it is set,
 * otherwise the reported value visibly walks through the intermediate levels.
 *
 * @return the target, or AIRFLOW_LEVEL_UNKNOWN / AIR_TEMP_LEVEL_UNKNOWN when idle
 */
vent_airflow_level_enum_t vent_button_control_pending_airflow(void);
vent_air_temp_level_enum_t vent_button_control_pending_air_temp(void);

#ifdef __cplusplus
}
#endif
