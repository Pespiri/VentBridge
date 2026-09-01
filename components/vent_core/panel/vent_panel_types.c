#include "vent_panel_types.h"

const char *vent_panel_airflow_level_name(vent_airflow_level_enum_t level) {
  switch (level) {
    case AIRFLOW_LEVEL_MIN: return "min";
    case AIRFLOW_LEVEL_NORM: return "norm";
    case AIRFLOW_LEVEL_MAX: return "max";
    case AIRFLOW_LEVEL_UNKNOWN:
    default: return "unknown";
  }
}

const char *vent_panel_air_temp_level_name(vent_air_temp_level_enum_t level) {
  switch (level) {
    case AIR_TEMP_LEVEL_NONE: return "none";
    case AIR_TEMP_LEVEL_LOW: return "low";
    case AIR_TEMP_LEVEL_LOW_MED: return "low/med";
    case AIR_TEMP_LEVEL_MED: return "med";
    case AIR_TEMP_LEVEL_MED_HIGH: return "med/high";
    case AIR_TEMP_LEVEL_HIGH: return "high";
    case AIR_TEMP_LEVEL_UNKNOWN:
    default: return "unknown";
  }
}
