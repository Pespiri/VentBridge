#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum VENT_AIRFLOW_LEVEL_ENUM {
  AIRFLOW_LEVEL_UNKNOWN = 0,
  AIRFLOW_LEVEL_MIN = 1,
  AIRFLOW_LEVEL_NORM = 2,
  AIRFLOW_LEVEL_MAX = 3,
} vent_airflow_level_enum_t;

typedef enum VENT_AIR_TEMP_LEVEL_ENUM {
  AIR_TEMP_LEVEL_UNKNOWN = -1,
  AIR_TEMP_LEVEL_NONE = 0,
  AIR_TEMP_LEVEL_LOW = 1,
  AIR_TEMP_LEVEL_LOW_MED = 2,
  AIR_TEMP_LEVEL_MED = 3,
  AIR_TEMP_LEVEL_MED_HIGH = 4,
  AIR_TEMP_LEVEL_HIGH = 5,
} vent_air_temp_level_enum_t;

typedef struct VENT_PANEL_STATE {
  vent_airflow_level_enum_t airflow_level;
  vent_air_temp_level_enum_t air_temp_level;
  bool sig_summer_operation;
  bool sig_heater_battery;
  bool sig_filter_change;
  uint16_t raw_value;
  uint16_t unknown_bits;
} vent_panel_state_t;

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Readable name for an airflow level */
const char *vent_panel_airflow_level_name(vent_airflow_level_enum_t level);

/** @brief Readable name for an air temperature level */
const char *vent_panel_air_temp_level_name(vent_air_temp_level_enum_t level);

#ifdef __cplusplus
}
#endif
