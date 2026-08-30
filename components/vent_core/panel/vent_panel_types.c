#include "vent_panel_types.h"

const char *vent_panel_fan_level_name(vent_fan_level_enum_t level) {
  switch (level) {
    case FAN_LEVEL_MIN: return "min";
    case FAN_LEVEL_NORM: return "norm";
    case FAN_LEVEL_MAX: return "max";
    case FAN_LEVEL_UNKNOWN:
    default: return "unknown";
  }
}

const char *vent_panel_temp_level_name(vent_temp_level_enum_t level) {
  switch (level) {
    case TEMP_LEVEL_NONE: return "none";
    case TEMP_LEVEL_LOW: return "low";
    case TEMP_LEVEL_LOW_MED: return "low/med";
    case TEMP_LEVEL_MED: return "med";
    case TEMP_LEVEL_MED_HIGH: return "med/high";
    case TEMP_LEVEL_HIGH: return "high";
    case TEMP_LEVEL_UNKNOWN:
    default: return "unknown";
  }
}
