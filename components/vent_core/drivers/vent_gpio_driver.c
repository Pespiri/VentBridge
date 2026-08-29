#include "vent_gpio_driver.h"

#include <driver/gpio.h>

esp_err_t vent_gpio_init_digital_output_pin(gpio_num_t pin) {
  if ((pin == GPIO_NUM_NC || pin >= GPIO_NUM_MAX)) return ESP_ERR_INVALID_ARG;

  esp_err_t err = gpio_config(&(gpio_config_t){
    .pin_bit_mask = (1 << pin),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE,
  });

  return err;
}

esp_err_t vent_gpio_set_state(gpio_num_t pin, uint8_t state) {
  if (pin == GPIO_NUM_NC) return ESP_OK;
  if (pin >= GPIO_NUM_MAX) return ESP_ERR_INVALID_STATE;
  return gpio_set_level(pin, state);
}
