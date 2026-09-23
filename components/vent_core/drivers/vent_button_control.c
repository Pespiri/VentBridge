#include "vent_button_control.h"

#include "../panel/vent_panel_reader.h"
#include "vent_gpio_driver.h"

#include <esp_log.h>
#include <freertos/queue.h>

static const char *const TAG = "vent_button_control";

#define BUTTON_PRESS_MS        200   // 200 milliseconds
#define BUTTON_STEP_GAP_MS     120   // 120 milliseconds
#define FILTER_LONG_TIMEOUT_MS 10000 // 10 seconds
#define FILTER_LONG_POLL_MS    50    // 50 milliseconds
#define MOVE_SETTLE_TIMEOUT_MS 2000  // 2 seconds
#define MOVE_SETTLE_POLL_MS    20    // 20 milliseconds

typedef enum BUTTON_CMD_TYPE {
  CMD_PRESS,
  CMD_MOVE_AIRFLOW,
  CMD_MOVE_AIR_TEMP,
} button_cmd_type_t;

typedef struct BUTTON_CMD {
  button_cmd_type_t type;
  union {
    vent_button_enum_t button;
    uint8_t target_level;
  };
} button_cmd_t;

static QueueHandle_t cmd_queue = NULL;
static vent_button_pins_t button_pins;

static volatile vent_airflow_level_enum_t pending_airflow = AIRFLOW_LEVEL_UNKNOWN;
static volatile vent_air_temp_level_enum_t pending_air_temp = AIR_TEMP_LEVEL_UNKNOWN;

/** @brief Get the GPIO pin associated with a given button
 *
 * @param[in]   button    button identifier
 *
 * @return      GPIO pin number corresponding to the button, or GPIO_NUM_NC if not applicable
 */
static gpio_num_t get_button_pin(vent_button_enum_t button);

/** @brief Generate a press pulse for a given button
 *
 * @param[in]   button    button to press
 * @param[in]   hold_ms   duration to hold the button pressed (in milliseconds)
 */
static void press_pulse(vent_button_enum_t button, uint32_t hold_ms);
/** @brief Hold the filter button until the panel state changes or a timeout occurs */
static void press_filter_long(void);

/** @brief Move the airflow to the specified target level
 *
 * @param[in]   target_level   desired airflow level
 */
static void move_airflow_to(vent_airflow_level_enum_t target_level);
/** @brief Move the air temperature to the specified target level
 *
 * @param[in]   target_level   desired air temperature level
 */
static void move_air_temp_to(vent_air_temp_level_enum_t target_level);

/** @brief Task function for handling button control commands */
static void vent_button_control_task(void *arg);

esp_err_t vent_button_control_init(const vent_button_pins_t *pins) {
  if (!pins) return ESP_ERR_INVALID_ARG;
  button_pins = *pins;

  const gpio_num_t pin_list[] = {
    button_pins.airflow_up,
    button_pins.airflow_down,
    button_pins.air_temp_up,
    button_pins.air_temp_down,
    button_pins.filter};
  for (uint8_t i = 0; i < sizeof(pin_list) / sizeof(pin_list[0]); i++) {
    esp_err_t err = vent_gpio_init_digital_output_pin(pin_list[i]);
    if (err != ESP_OK) return err;
    vent_gpio_set_state(pin_list[i], VENT_GPIO_LOW);
  }

  cmd_queue = xQueueCreate(4, sizeof(button_cmd_t));
  return cmd_queue ? ESP_OK : ESP_ERR_NO_MEM;
}

void vent_button_control_start_task(UBaseType_t priority) {
  xTaskCreate(vent_button_control_task, TAG, 4096, NULL, priority, NULL);
}

esp_err_t vent_button_control_press(vent_button_enum_t button) {
  button_cmd_t cmd = {.type = CMD_PRESS, .button = button};
  return xQueueSend(cmd_queue, &cmd, 0) == pdTRUE ? ESP_OK : ESP_ERR_INVALID_STATE;
}

esp_err_t vent_button_control_move_airflow_to(vent_airflow_level_enum_t target_level) {
  button_cmd_t cmd = {.type = CMD_MOVE_AIRFLOW, .target_level = (uint8_t)target_level};
  if (xQueueSend(cmd_queue, &cmd, 0) != pdTRUE) return ESP_ERR_INVALID_STATE;
  pending_airflow = target_level;
  return ESP_OK;
}

esp_err_t vent_button_control_move_air_temp_to(vent_air_temp_level_enum_t target_level) {
  button_cmd_t cmd = {.type = CMD_MOVE_AIR_TEMP, .target_level = (uint8_t)target_level};
  if (xQueueSend(cmd_queue, &cmd, 0) != pdTRUE) return ESP_ERR_INVALID_STATE;
  pending_air_temp = target_level;
  return ESP_OK;
}

vent_airflow_level_enum_t vent_button_control_pending_airflow(void) {
  return pending_airflow;
}

vent_air_temp_level_enum_t vent_button_control_pending_air_temp(void) {
  return pending_air_temp;
}

static gpio_num_t get_button_pin(vent_button_enum_t button) {
  switch (button) {
    // airflow control buttons
    case BUTTON_AIRFLOW_UP: return button_pins.airflow_up;
    case BUTTON_AIRFLOW_DOWN: return button_pins.airflow_down;

    // air temperature control buttons
    case BUTTON_AIR_TEMP_UP: return button_pins.air_temp_up;
    case BUTTON_AIR_TEMP_DOWN: return button_pins.air_temp_down;

    // filter control buttons
    case BUTTON_FILTER_LONG: return button_pins.filter;

    default: return GPIO_NUM_NC;
  }
}

static void press_pulse(vent_button_enum_t button, uint32_t hold_ms) {
  gpio_num_t pin = get_button_pin(button);
  if (pin == GPIO_NUM_NC) return;

  ESP_LOGD(TAG, "pulse button %d on GPIO %d for %lums", (int)button, (int)pin, (unsigned long)hold_ms);
  vent_gpio_set_state(pin, VENT_GPIO_HIGH);
  vTaskDelay(pdMS_TO_TICKS(hold_ms));
  vent_gpio_set_state(pin, VENT_GPIO_LOW);
}

static void press_filter_long(void) {
  vent_panel_state_t start_state = vent_panel_reader_get_state();
  gpio_num_t pin = get_button_pin(BUTTON_FILTER_LONG);
  vent_gpio_set_state(pin, VENT_GPIO_HIGH);

  uint32_t waited_ms = 0;
  while (waited_ms < FILTER_LONG_TIMEOUT_MS) {
    vTaskDelay(pdMS_TO_TICKS(FILTER_LONG_POLL_MS));
    waited_ms += FILTER_LONG_POLL_MS;
    if (vent_panel_reader_get_state().raw_value != start_state.raw_value) {
      break;
    }
  }

  vent_gpio_set_state(pin, VENT_GPIO_LOW);
  ESP_LOGD(TAG, "filter long press released after %lums", (unsigned long)waited_ms);
}

static void move_airflow_to(vent_airflow_level_enum_t target_level) {
  if (target_level < AIRFLOW_LEVEL_MIN || target_level > AIRFLOW_LEVEL_MAX) return;
  vent_airflow_level_enum_t current = vent_panel_reader_get_state().airflow_level;
  if (current == AIRFLOW_LEVEL_UNKNOWN || current == target_level) return;

  int steps = current > target_level ? current - target_level : target_level - current;
  vent_button_enum_t direction = target_level > current ? BUTTON_AIRFLOW_UP : BUTTON_AIRFLOW_DOWN;
  for (int i = 0; i < steps; i++) {
    press_pulse(direction, BUTTON_PRESS_MS);
    vTaskDelay(pdMS_TO_TICKS(BUTTON_STEP_GAP_MS));
  }

  // final level is only known once the panel reports it; returning earlier
  // would let consumers publish the second-to-last level as if the move ended
  for (uint32_t waited_ms = 0; waited_ms < MOVE_SETTLE_TIMEOUT_MS; waited_ms += MOVE_SETTLE_POLL_MS) {
    if (vent_panel_reader_get_state().airflow_level == target_level) return;
    vTaskDelay(pdMS_TO_TICKS(MOVE_SETTLE_POLL_MS));
  }
  ESP_LOGW(TAG, "airflow did not reach level %d", (int)target_level);
}

static void move_air_temp_to(vent_air_temp_level_enum_t target_level) {
  if (target_level < AIR_TEMP_LEVEL_NONE || target_level > AIR_TEMP_LEVEL_HIGH) return;
  vent_air_temp_level_enum_t current = vent_panel_reader_get_state().air_temp_level;
  if (current == AIR_TEMP_LEVEL_UNKNOWN || current == target_level) return;

  int steps = current > target_level ? current - target_level : target_level - current;
  vent_button_enum_t direction = target_level > current ? BUTTON_AIR_TEMP_UP : BUTTON_AIR_TEMP_DOWN;
  for (int i = 0; i < steps; i++) {
    press_pulse(direction, BUTTON_PRESS_MS);
    vTaskDelay(pdMS_TO_TICKS(BUTTON_STEP_GAP_MS));
  }

  // final level is only known once the panel reports it; returning earlier
  // would let consumers publish the second-to-last level as if the move ended
  for (uint32_t waited_ms = 0; waited_ms < MOVE_SETTLE_TIMEOUT_MS; waited_ms += MOVE_SETTLE_POLL_MS) {
    if (vent_panel_reader_get_state().air_temp_level == target_level) return;
    vTaskDelay(pdMS_TO_TICKS(MOVE_SETTLE_POLL_MS));
  }
  ESP_LOGW(TAG, "temperature did not reach level %d", (int)target_level);
}

static void vent_button_control_task(void *arg) {
  (void)arg;
  button_cmd_t cmd;
  for (;;) {
    if (xQueueReceive(cmd_queue, &cmd, portMAX_DELAY) != pdTRUE) continue;

    switch (cmd.type) {
      case CMD_PRESS: {
        if (cmd.button == BUTTON_FILTER_LONG) {
          press_filter_long();
        } else {
          press_pulse(cmd.button, BUTTON_PRESS_MS);
        }
        vTaskDelay(pdMS_TO_TICKS(BUTTON_STEP_GAP_MS));
        break;
      }
      case CMD_MOVE_AIRFLOW: {
        move_airflow_to((vent_airflow_level_enum_t)cmd.target_level);
        if (pending_airflow == (vent_airflow_level_enum_t)cmd.target_level) {
          pending_airflow = AIRFLOW_LEVEL_UNKNOWN;
        }
        break;
      }
      case CMD_MOVE_AIR_TEMP: {
        move_air_temp_to((vent_air_temp_level_enum_t)cmd.target_level);
        if (pending_air_temp == (vent_air_temp_level_enum_t)cmd.target_level) {
          pending_air_temp = AIR_TEMP_LEVEL_UNKNOWN;
        }
        break;
      }
    }
  }
}
