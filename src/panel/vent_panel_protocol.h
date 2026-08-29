#pragma once

#include "vent_panel_types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Villavent/Systemair VR panel-bus status frame: FF 01 b2 b3 FF crc 00
// (CRC-8/MAXIM over bytes [0..4]). b2|b3<<8 forms a 16-bit state bitmap
#define VENT_PANEL_STATUS_FRAME_LEN 7

#ifdef __cplusplus
extern "C" {
#endif

/** @brief CRC-8/MAXIM (poly 0x8C, init 0x00), as used on the panel bus */
uint8_t vent_panel_protocol_crc8(const uint8_t *data, size_t len);

/**
 * @brief Decode a raw panel-bus status frame into `out_state`
 *
 * @return true if `frame` has a valid header and checksum; false otherwise
 *         (in which case `out_state` is left unmodified)
 */
bool vent_panel_protocol_decode_status(const uint8_t *frame, size_t len, vent_panel_state_t *out_state);

#ifdef __cplusplus
}
#endif
