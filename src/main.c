#include "console/vent_console.h"
#include "drivers/vent_button_control.h"
#include "panel/vent_panel_reader.h"
#include "project_config.h"
#include "project_meta.h"
#include "utilities/log_utils.h"

static const char *TAG = "main";

void app_main(void);

void app_main(void) {
  static const vent_panel_reader_config_t panel_config = VENT_PANEL_READER_CONFIG_DEFAULT;
  static const vent_button_pins_t button_pins = VENT_BUTTON_PINS_DEFAULT;

  LOGN(TAG, "-------------   META   -------------");
  LOGN(TAG, "name:           " PROJECT_NAME);
  LOGN(TAG, "firmware:       " FW_VERSION);
  LOGN(TAG, "------------------------------------");

  ESP_ERROR_CHECK(vent_panel_reader_init(&panel_config));
  ESP_ERROR_CHECK(vent_button_control_init(&button_pins));

  vent_panel_reader_start_task(TASK_PRIORITY_UART_READER);
  vent_button_control_start_task(TASK_PRIORITY_BUTTON_CONTROL);

  LOGN(TAG, "%s v%s ready", PROJECT_NAME, FW_VERSION);

  ESP_ERROR_CHECK(vent_console_start(TASK_PRIORITY_CONSOLE));
}
