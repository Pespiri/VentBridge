#pragma once

#include <esp_err.h>
#include <hal/gpio_types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief RF switch wiring on boards that can choose between two antennas */
typedef struct VENT_ANTENNA_PINS {
  gpio_num_t rf_switch_enable; // held LOW to power the RF switch
  gpio_num_t select;           // LOW = onboard antenna, HIGH = external connector
} vent_antenna_pins_t;

/**
 * @brief Point the RF switch at one of the two antennas
 *
 * Both pins stay driven for as long as the board is powered; the RF switch loses
 * its path the moment the enable line is released. Call before the radio starts.
 *
 * @param[in]   pins       wiring, or GPIO_NUM_NC on either pin for a fixed-antenna board
 * @param[in]   external   true to route RF to the external connector
 */
esp_err_t vent_antenna_select(const vent_antenna_pins_t *pins, bool external);

#ifdef __cplusplus
}
#endif
