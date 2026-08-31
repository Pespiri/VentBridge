# VentBridge

Home Assistant control for a **Systemair Villavent VR400E** heat-recovery ventilation
unit, by tapping the wired control panel's bus with an ESP32.

The unit has no network interface — the only thing it talks to is its wall panel. VentBridge
listens on that bus, decodes the panel protocol, and exposes fan speed, heat recovery,
filter status and summer mode as native Home Assistant entities over WiFi.

---

## How it works

The panel bus is **UART framing over a CAN physical layer** — a differential transceiver is
used purely for signalling; there is no CAN controller or arbitration involved. A plain
ESP32 UART reads it directly at 4800 baud 8N1.

```
  VR400E  <--- CAN-PHY differential pair --->  wall panel
              |
              +--> CAN transceiver --> ESP32 UART RX   (status frames in)
                                       ESP32 GPIO x5   (button contacts out)
```

State is **read** by decoding the bus. Buttons are **actuated** by pulsing GPIO lines wired
in parallel with the panel's physical button contacts.

---

## Two firmwares, one driver

| | Purpose | Build |
| --- | --- | --- |
| **ESPHome** | Deployed firmware. WiFi, Home Assistant API, OTA. | `pio run -t esphome_compile` |
| **ESP-IDF app** | Bench tool. USB serial console for probing the bus. | `pio run` |

Both compile the same `vent_core` driver, so protocol work benefits both.

```
components/
  vent_core/            reusable ESP-IDF component - the actual driver
    panel/              frame decoding, state model
    drivers/            UART + GPIO button control
  vent_bridge/          ESPHome external component (C++ / codegen)
    select/ button/ binary_sensor/ switch/
src/                    ESP-IDF app: main, USB console, board pin map
esphome/                ESPHome project YAML
scripts/                PlatformIO -> ESPHome CLI glue
```

---

## Quick start

### ESPHome (the one you flash)

```bash
cp esphome/secrets.yaml.example esphome/secrets.yaml   # then fill it in
python3 -m venv .venv-esphome && .venv-esphome/bin/pip install esphome

pio run -t esphome_compile
ESPHOME_DEVICE=/dev/ttyACM0     pio run -t esphome_upload   # first flash, over USB
ESPHOME_DEVICE=ventbridge.local pio run -t esphome_upload   # thereafter, OTA
pio run -t esphome_logs
```

Generate the API key with `openssl rand -base64 32`.

The component can also be consumed from a standalone ESPHome project — the YAML does not
need to live in this repo:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/Pespiri/VentBridge
      ref: master
    components: [vent_bridge]
```

`vent_bridge` pulls in `vent_core` and its required `sdkconfig` options by itself, so the
consuming YAML stays minimal. Add `refresh: 0s` while iterating; ESPHome caches git sources
for a day.

### ESP-IDF console app

```bash
pio run && pio run -t upload && pio device monitor
```

---

## Entities

| Entity | Type | Notes |
| --- | --- | --- |
| Fan speed | `select` | `min` / `norm` / `max` |
| Heat recovery | `select` | `none` / `low` / `low-med` / `med` / `med-high` / `high` |
| Fan up / down | `button` | single step |
| Temperature up / down | `button` | single step |
| Filter reset | `button` | long press; clears the filter light |
| Summer mode | `binary_sensor` | read-only |
| Filter alarm | `binary_sensor` | `problem` |
| Panel online | `binary_sensor` | `connectivity` |
| Frame trace | `switch` | optional; commented out in the YAML |

Level changes are performed as repeated single-step presses, so the panel reports every
intermediate level. The driver reports the *target* while a move is in flight and only
publishes the real level once the panel confirms it, so the UI doesn't walk through the
values on its way to the destination.

---

## Console commands

USB serial, ESP-IDF build only.

| Command | Description |
| --- | --- |
| `state` | Decoded panel state plus the raw bitmap |
| `fan <min\|norm\|max>` | Move fan to a level |
| `temp <0-5>` | Move heat recovery to a level |
| `press <fanup\|fandown\|tempup\|tempdown\|filterlong>` | Single button press |
| `trace [on\|off]` | Hex-dump bus frames |

---

## Protocol

### Status frame — ventilator to bus, every ~100 ms

```
FF 01 <lo> <hi> FF <crc> 00          7 bytes
```

`lo | hi<<8` is a 16-bit state bitmap. `crc` is **CRC-8/MAXIM** (poly `0x8C`, init `0x00`)
over bytes `[0..4]`. Unrecognised bits are surfaced as `unknown_bits` in `state`, which is
how the remaining fields get mapped.

### Button frame — panel to bus

Sent in the slot **immediately after** a status frame, so a capture shows the two merged
into one 13-byte burst:

```
FF 01 92 00 FF 14 00 | 00 10 00 FF 7F 00
\------ status -----/ \----- button ----/
```

Same CRC. The payload is a bitmask:

| Bit | Button |
| --- | --- |
| `b1` bit 1 | temperature down |
| `b1` bit 2 | temperature up |
| `b1` bit 3 | fan down |
| `b1` bit 4 | fan up |
| `b2` bit 1 | filter (long) |

> **Note** — button frames are decoded and understood, but VentBridge does **not** transmit
> them; it uses GPIO contacts instead.

### Frame tracing

`trace on` (or the `Frame trace` switch) hex-dumps every chunk, including ones the decoder
rejects — which is exactly what you need for mapping unknown bits. Identical consecutive
frames are collapsed, with a periodic progress line and an exact total when the frame
changes:

```
rx 7 bytes
ff 01 92 00 ff 14 00
(same frame seen 100 times so far)
(last frame seen 137 times)
rx 7 bytes
ff 01 12 01 ff b2 00
```

---

## Hardware

Currently a **Seeed XIAO ESP32-S3**; target is an **ESP32-C6**. Switch with the `board`
substitution in `esphome/ventbridge.yaml` and `board` in `platformio.ini`.

| Signal | GPIO |
| --- | --- |
| Panel UART RX | 8 |
| Panel UART TX | 9 |
| Fan up / down | 2 / 3 |
| Temperature up / down | 4 / 5 |
| Filter | 6 |

Pins are configured in YAML for the ESPHome build and in `src/project_config.h` for the
ESP-IDF app; keep them in step.

---

## Versioning

`project.version` is taken from `git describe --tags`, so GitHub releases drive the firmware
version shown in Home Assistant. A `-dirty` suffix means it was built from a working copy
rather than a tagged commit. Override with `ESPHOME_VERSION=1.2.3`.
