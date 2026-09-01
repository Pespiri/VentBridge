#include "vent_console.h"

#include "drivers/vent_button_control.h"
#include "panel/vent_panel_reader.h"
#include "project_meta.h"
#include "utilities/log_utils.h"

#include <esp_console.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "vent_console";

#define CONSOLE_OK              0
#define CONSOLE_ERROR           1

#define CONSOLE_MAX_CMDLINE_LEN 128
#define CONSOLE_MAX_HISTORY_LEN 16

/** @brief Map an airflow level name to its enum, or AIRFLOW_LEVEL_UNKNOWN if unrecognised */
static vent_airflow_level_enum_t airflow_level_from_name(const char *name);
/** @brief Map a button name to a vent_button_enum_t, or -1 if unrecognised */
static int button_from_name(const char *name);
/** @brief Report the outcome of a queued button command */
static int report_queue_result(esp_err_t err);

static int cmd_state(int argc, char **argv);
static int cmd_airflow(int argc, char **argv);
static int cmd_air_temp(int argc, char **argv);
static int cmd_press(int argc, char **argv);
static int cmd_trace(int argc, char **argv);

/** @brief Register a single command with the console */
static esp_err_t register_command(const char *command, const char *help, const char *hint, esp_console_cmd_func_t func);

esp_err_t vent_console_start(UBaseType_t priority) {
  esp_console_repl_t *repl = NULL;

  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.prompt = PROJECT_NAME ">";
  repl_config.max_cmdline_length = CONSOLE_MAX_CMDLINE_LEN;
  repl_config.max_history_len = CONSOLE_MAX_HISTORY_LEN;
  repl_config.task_priority = priority;

  esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();

  esp_err_t err = esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl);
  if (err != ESP_OK) {
    LOGE(TAG, "console init failed: %s", esp_err_to_name(err));
    return err;
  }

  err = esp_console_register_help_command();
  if (err != ESP_OK) return err;

  err = register_command("state", "Show the last decoded panel state", NULL, cmd_state);
  if (err != ESP_OK) return err;
  err = register_command("air", "Move the airflow to a target level", "<min|norm|max>", cmd_airflow);
  if (err != ESP_OK) return err;
  err = register_command("temp", "Move the temperature to a target level", "<0-5>", cmd_air_temp);
  if (err != ESP_OK) return err;
  err = register_command("press", "Press a single button", "<airup|airdown|tempup|tempdown|filterlong>", cmd_press);
  if (err != ESP_OK) return err;
  err = register_command("trace", "Hex-dump received panel frames", "[on|off]", cmd_trace);
  if (err != ESP_OK) return err;
  LOGN(TAG, "console ready, type 'help' for commands");
  return esp_console_start_repl(repl);
}

static esp_err_t register_command(const char *command, const char *help, const char *hint, esp_console_cmd_func_t func) {
  const esp_console_cmd_t cmd = {
    .command = command,
    .help = help,
    .hint = hint,
    .func = func,
  };
  return esp_console_cmd_register(&cmd);
}

static vent_airflow_level_enum_t airflow_level_from_name(const char *name) {
  if (!strcasecmp(name, "min")) return AIRFLOW_LEVEL_MIN;
  if (!strcasecmp(name, "norm")) return AIRFLOW_LEVEL_NORM;
  if (!strcasecmp(name, "max")) return AIRFLOW_LEVEL_MAX;
  return AIRFLOW_LEVEL_UNKNOWN;
}

static int button_from_name(const char *name) {
  if (!strcasecmp(name, "airup")) return BUTTON_AIRFLOW_UP;
  if (!strcasecmp(name, "airdown")) return BUTTON_AIRFLOW_DOWN;
  if (!strcasecmp(name, "tempup")) return BUTTON_AIR_TEMP_UP;
  if (!strcasecmp(name, "tempdown")) return BUTTON_AIR_TEMP_DOWN;
  if (!strcasecmp(name, "filterlong")) return BUTTON_FILTER_LONG;
  return -1;
}

static int report_queue_result(esp_err_t err) {
  if (err != ESP_OK) {
    printf("command queue full, try again\n");
    return CONSOLE_ERROR;
  }
  printf("queued\n");
  return CONSOLE_OK;
}

static int cmd_state(int argc, char **argv) {
  (void)argc;
  (void)argv;

  vent_panel_state_t state = vent_panel_reader_get_state();

  printf("bus:      %s\n", vent_panel_reader_is_online() ? "online" : "offline");
  printf("airflow:  %s (%d)\n", vent_panel_airflow_level_name(state.airflow_level), (int)state.airflow_level);
  printf("air temp: %s (%d)\n", vent_panel_air_temp_level_name(state.air_temp_level), (int)state.air_temp_level);
  printf("summer:   %s\n", state.summer_on ? "on" : "off");
  printf("notify:   %s\n", state.notification_on ? "on" : "off");
  printf("raw:      0x%04X (unknown bits 0x%04X)\n", state.raw_value, state.unknown_bits);
  return CONSOLE_OK;
}

static int cmd_airflow(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: air <min|norm|max>\n");
    return CONSOLE_ERROR;
  }

  vent_airflow_level_enum_t level = airflow_level_from_name(argv[1]);
  if (level == AIRFLOW_LEVEL_UNKNOWN) {
    printf("unknown air level '%s', expected min|norm|max\n", argv[1]);
    return CONSOLE_ERROR;
  }

  return report_queue_result(vent_button_control_move_airflow_to(level));
}

static int cmd_air_temp(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: temp <0-5>\n");
    return CONSOLE_ERROR;
  }

  char *end = NULL;
  long level = strtol(argv[1], &end, 10);
  if (end == argv[1] || *end != '\0' || level < AIR_TEMP_LEVEL_NONE || level > AIR_TEMP_LEVEL_HIGH) {
    printf("temperature level must be an integer 0-5\n");
    return CONSOLE_ERROR;
  }

  return report_queue_result(vent_button_control_move_air_temp_to((vent_air_temp_level_enum_t)level));
}

static int cmd_press(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: press <airup|airdown|tempup|tempdown|filterlong>\n");
    return CONSOLE_ERROR;
  }

  int button = button_from_name(argv[1]);
  if (button < 0) {
    printf("unknown button '%s'\n", argv[1]);
    return CONSOLE_ERROR;
  }

  return report_queue_result(vent_button_control_press((vent_button_enum_t)button));
}

static int cmd_trace(int argc, char **argv) {
  if (argc == 2) {
    if (!strcasecmp(argv[1], "on")) {
      vent_panel_reader_set_trace(true);
    } else if (!strcasecmp(argv[1], "off")) {
      vent_panel_reader_set_trace(false);
    } else {
      printf("usage: trace [on|off]\n");
      return CONSOLE_ERROR;
    }
  } else if (argc != 1) {
    printf("usage: trace [on|off]\n");
    return CONSOLE_ERROR;
  }

  printf("trace: %s\n", vent_panel_reader_get_trace() ? "on" : "off");
  return CONSOLE_OK;
}
