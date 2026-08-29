#include "console/vent_console.h"
#include "drivers/vent_button_control.h"
#include "panel/vent_panel_reader.h"
#include "project_config.h"
#include "project_meta.h"
#include "utilities/log_utils.h"

static const char *MAIN_TAG = "main";

void app_main(void);

void app_main(void) {
  LOGN(MAIN_TAG, "-------------   META   -------------");
  LOGN(MAIN_TAG, "name:           " PROJECT_NAME);
  LOGN(MAIN_TAG, "firmware:       " FW_VERSION);
  LOGN(MAIN_TAG, "------------------------------------");

  ESP_ERROR_CHECK(vent_panel_reader_init());
  ESP_ERROR_CHECK(vent_button_control_init());

  vent_panel_reader_start_task(TASK_PRIORITY_UART_READER);
  vent_button_control_start_task(TASK_PRIORITY_BUTTON_CONTROL);

  LOGN(MAIN_TAG, "%s v%s ready log_utils", PROJECT_NAME, FW_VERSION);

  ESP_ERROR_CHECK(vent_console_start(TASK_PRIORITY_CONSOLE));
}
