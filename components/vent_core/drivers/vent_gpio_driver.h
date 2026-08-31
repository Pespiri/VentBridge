#pragma once

#include <esp_err.h>
#include <hal/gpio_types.h>
#include <stdint.h>

#define VENT_GPIO_LOW  (0)
#define VENT_GPIO_HIGH (1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Assign new GPIO pin
 *
 * @param[in]   pin     pin to initialize
 */
esp_err_t vent_gpio_init_digital_output_pin(gpio_num_t pin);
/**
 * @brief Set GPIO pin state
 *
 * @param[in]   pin     pin
 * @param[in]   state   desired pin state [0,1]
 */
esp_err_t vent_gpio_set_state(gpio_num_t pin, uint8_t state);

#ifdef __cplusplus
}
#endif
