#include "vent_panel_protocol.h"

// #define FILTER_ON_BIT       1 << ? // not observed (yet)
#define SUMMER_ON_BIT       1 << 4 // 0b00010000
#define NOTIFICATION_ON_BIT 1 << 5 // 0b00100000

/** @brief Map the air temperature bits of the state bitmap to an air temperature level */
static vent_air_temp_level_enum_t decode_air_temp_level(uint16_t value);
/** @brief Map the airflow bits of the state bitmap to an airflow level */
static vent_airflow_level_enum_t decode_airflow_level(uint16_t value);

uint8_t vent_panel_protocol_crc8(const uint8_t *data, size_t len) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++) {
      crc = (crc & 0x01) ? (uint8_t)((crc >> 1) ^ 0x8C) : (uint8_t)(crc >> 1);
    }
  }
  return crc;
}

bool vent_panel_protocol_decode_status(const uint8_t *frame, size_t len, vent_panel_state_t *out_state) {
  if (len < VENT_PANEL_STATUS_FRAME_LEN) return false;
  if (frame[0] != 0xFF || frame[1] != 0x01 || frame[4] != 0xFF) return false;
  if (vent_panel_protocol_crc8(frame, 5) != frame[5]) return false;

  uint16_t value = (uint16_t)frame[2] | ((uint16_t)frame[3] << 8);
  out_state->airflow_level = decode_airflow_level(value);
  out_state->air_temp_level = decode_air_temp_level(value);
  out_state->summer_on = value & SUMMER_ON_BIT;
  out_state->notification_on = value & NOTIFICATION_ON_BIT;
  out_state->raw_value = value;
  out_state->unknown_bits = value & (uint16_t)~0x01FE;
  return true;
}

bool vent_panel_protocol_decode_button(const uint8_t *frame, size_t len, uint16_t *out_buttons) {
  if (len < VENT_PANEL_BUTTON_FRAME_LEN) return false;
  if (frame[0] != 0x00 || frame[3] != 0xFF) return false;
  if (vent_panel_protocol_crc8(frame, 4) != frame[4]) return false;

  *out_buttons = (uint16_t)frame[1] | ((uint16_t)frame[2] << 8);
  return true;
}

static vent_air_temp_level_enum_t decode_air_temp_level(uint16_t value) {
  bool low = value & (1 << 1), medium = value & (1 << 2), high = value & (1 << 3);
  if (!low && !medium && !high) return AIR_TEMP_LEVEL_NONE;
  if (low && !medium && !high) return AIR_TEMP_LEVEL_LOW;
  if (low && medium && !high) return AIR_TEMP_LEVEL_LOW_MED;
  if (!low && medium && !high) return AIR_TEMP_LEVEL_MED;
  if (!low && medium && high) return AIR_TEMP_LEVEL_MED_HIGH;
  if (!low && !medium && high) return AIR_TEMP_LEVEL_HIGH;
  return AIR_TEMP_LEVEL_UNKNOWN;
}

static vent_airflow_level_enum_t decode_airflow_level(uint16_t value) {
  if (value & (1 << 6)) return AIRFLOW_LEVEL_MIN;
  if (value & (1 << 7)) return AIRFLOW_LEVEL_NORM;
  if (value & (1 << 8)) return AIRFLOW_LEVEL_MAX;
  return AIRFLOW_LEVEL_UNKNOWN;
}
