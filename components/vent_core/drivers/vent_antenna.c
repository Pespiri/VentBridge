#include "vent_antenna.h"

#include "vent_gpio_driver.h"

#include <esp_log.h>

static const char *const TAG = "vent_antenna";

esp_err_t vent_antenna_select(const vent_antenna_pins_t *pins, bool external) {
  if (!pins) return ESP_ERR_INVALID_ARG;
  if (pins->rf_switch_enable == GPIO_NUM_NC || pins->select == GPIO_NUM_NC) return ESP_OK;

  esp_err_t err = vent_gpio_init_digital_output_pin(pins->rf_switch_enable);
  if (err != ESP_OK) return err;
  err = vent_gpio_init_digital_output_pin(pins->select);
  if (err != ESP_OK) return err;

  // select has to settle before the switch is powered, or the RF path glitches
  err = vent_gpio_set_state(pins->select, external ? VENT_GPIO_HIGH : VENT_GPIO_LOW);
  if (err != ESP_OK) return err;
  err = vent_gpio_set_state(pins->rf_switch_enable, VENT_GPIO_LOW);
  if (err != ESP_OK) return err;

  ESP_LOGI(TAG, "%s antenna selected (enable GPIO %d, select GPIO %d)", external ? "external" : "onboard", (int)pins->rf_switch_enable, (int)pins->select);
  return ESP_OK;
}
