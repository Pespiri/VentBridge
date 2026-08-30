#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum VENT_FAN_LEVEL_ENUM {
  FAN_LEVEL_UNKNOWN = 0,
  FAN_LEVEL_MIN = 1,
  FAN_LEVEL_NORM = 2,
  FAN_LEVEL_MAX = 3,
} vent_fan_level_enum_t;

typedef enum VENT_TEMP_LEVEL_ENUM {
  TEMP_LEVEL_UNKNOWN = -1,
  TEMP_LEVEL_NONE = 0,
  TEMP_LEVEL_LOW = 1,
  TEMP_LEVEL_LOW_MED = 2,
  TEMP_LEVEL_MED = 3,
  TEMP_LEVEL_MED_HIGH = 4,
  TEMP_LEVEL_HIGH = 5,
} vent_temp_level_enum_t;

typedef struct VENT_PANEL_STATE {
  vent_fan_level_enum_t fan_level;
  vent_temp_level_enum_t temp_level;
  bool summer_on;
  bool filter_on;
  uint16_t raw_value;
  uint16_t unknown_bits;
} vent_panel_state_t;

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Readable name for a fan level */
const char *vent_panel_fan_level_name(vent_fan_level_enum_t level);

/** @brief Readable name for a temperature level */
const char *vent_panel_temp_level_name(vent_temp_level_enum_t level);

#ifdef __cplusplus
}
#endif
