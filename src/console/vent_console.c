#include "vent_console.h"

#include "../drivers/vent_button_control.h"
#include "../panel/vent_panel_reader.h"
#include "../project_meta.h"
#include "../utilities/log_utils.h"

#include <esp_console.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VENT_CONSOLE_TAG        "vent_console"

#define CONSOLE_MAX_CMDLINE_LEN 128
#define CONSOLE_MAX_HISTORY_LEN 16

/** @brief Map a fan level name to its enum, or FAN_LEVEL_UNKNOWN if unrecognised */
static vent_fan_level_enum_t fan_level_from_name(const char *name);
/** @brief Map a button name to a vent_button_enum_t, or -1 if unrecognised */
static int button_from_name(const char *name);
/** @brief Report the outcome of a queued button command */
static int report_queue_result(esp_err_t err);

static int cmd_state(int argc, char **argv);
static int cmd_fan(int argc, char **argv);
static int cmd_temp(int argc, char **argv);
static int cmd_press(int argc, char **argv);

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
    LOGE(VENT_CONSOLE_TAG, "console init failed: %s", esp_err_to_name(err));
    return err;
  }

  err = esp_console_register_help_command();
  if (err != ESP_OK) return err;

  err = register_command("state", "Show the last decoded panel state", NULL, cmd_state);
  if (err != ESP_OK) return err;
  err = register_command("fan", "Move the fan to a target level", "<low|norm|high>", cmd_fan);
  if (err != ESP_OK) return err;
  err = register_command("temp", "Move the temperature to a target level", "<0-5>", cmd_temp);
  if (err != ESP_OK) return err;
  err = register_command("press", "Press a single button", "<fanup|fandown|tempup|tempdown|filter|filterlong>", cmd_press);
  if (err != ESP_OK) return err;

  LOGN(VENT_CONSOLE_TAG, "console ready, type 'help' for commands");
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

static vent_fan_level_enum_t fan_level_from_name(const char *name) {
  if (!strcasecmp(name, "low")) return FAN_LEVEL_LOW;
  if (!strcasecmp(name, "norm")) return FAN_LEVEL_NORM;
  if (!strcasecmp(name, "high")) return FAN_LEVEL_HIGH;
  return FAN_LEVEL_UNKNOWN;
}

static int button_from_name(const char *name) {
  if (!strcasecmp(name, "fanup")) return BUTTON_FAN_UP;
  if (!strcasecmp(name, "fandown")) return BUTTON_FAN_DOWN;
  if (!strcasecmp(name, "tempup")) return BUTTON_TEMP_UP;
  if (!strcasecmp(name, "tempdown")) return BUTTON_TEMP_DOWN;
  if (!strcasecmp(name, "filter")) return BUTTON_FILTER;
  if (!strcasecmp(name, "filterlong")) return BUTTON_FILTER_LONG;
  return -1;
}

static int report_queue_result(esp_err_t err) {
  if (err != ESP_OK) {
    printf("command queue full, try again\n");
    return 1;
  }
  printf("queued\n");
  return 0;
}

static int cmd_state(int argc, char **argv) {
  (void)argc;
  (void)argv;

  vent_panel_state_t state = vent_panel_reader_get_state();

  printf("bus:     %s\n", vent_panel_reader_is_online() ? "online" : "offline");
  printf("fan:     %s (%d)\n", vent_panel_fan_level_name(state.fan_level), (int)state.fan_level);
  printf("temp:    %s (%d)\n", vent_panel_temp_level_name(state.temp_level), (int)state.temp_level);
  printf("summer:  %s\n", state.summer_on ? "on" : "off");
  printf("filter:  %s\n", state.filter_on ? "on" : "off");
  printf("raw:     0x%04X (unknown bits 0x%04X)\n", state.raw_value, state.unknown_bits);
  return 0;
}

static int cmd_fan(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: fan <low|norm|high>\n");
    return 1;
  }

  vent_fan_level_enum_t level = fan_level_from_name(argv[1]);
  if (level == FAN_LEVEL_UNKNOWN) {
    printf("unknown fan level '%s', expected low|norm|high\n", argv[1]);
    return 1;
  }

  return report_queue_result(vent_button_control_move_fan_to(level));
}

static int cmd_temp(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: temp <0-5>\n");
    return 1;
  }

  char *end = NULL;
  long level = strtol(argv[1], &end, 10);
  if (end == argv[1] || *end != '\0' || level < TEMP_LEVEL_NONE || level > TEMP_LEVEL_HIGH) {
    printf("temperature level must be an integer 0-5\n");
    return 1;
  }

  return report_queue_result(vent_button_control_move_temp_to((vent_temp_level_enum_t)level));
}

static int cmd_press(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: press <fanup|fandown|tempup|tempdown|filter|filterlong>\n");
    return 1;
  }

  int button = button_from_name(argv[1]);
  if (button < 0) {
    printf("unknown button '%s'\n", argv[1]);
    return 1;
  }

  return report_queue_result(vent_button_control_press((vent_button_enum_t)button));
}
