import os

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components.esp32 import add_idf_component, add_idf_sdkconfig_option
from esphome.const import CONF_ID, CONF_BAUD_RATE, CONF_RX_PIN, CONF_TX_PIN

CODEOWNERS = ["@yoga"]
MULTI_CONF = False
DEPENDENCIES = ["esp32"]

# The driver core is a plain ESP-IDF component living beside this one.
VENT_CORE_PATH = os.path.normpath(
    os.path.join(os.path.dirname(__file__), "..", "vent_core")
)

vent_bridge_ns = cg.esphome_ns.namespace("vent_bridge")
VentBridge = vent_bridge_ns.class_("VentBridge", cg.Component)

CONF_VENT_BRIDGE_ID = "vent_bridge_id"
CONF_UART_PORT = "uart_port"
CONF_AIRFLOW_UP_PIN = "airflow_up_pin"
CONF_AIRFLOW_DOWN_PIN = "airflow_down_pin"
CONF_AIR_TEMP_UP_PIN = "air_temp_up_pin"
CONF_AIR_TEMP_DOWN_PIN = "air_temp_down_pin"
CONF_FILTER_PIN = "filter_pin"

_BUTTON_PINS = (
    CONF_AIRFLOW_UP_PIN,
    CONF_AIRFLOW_DOWN_PIN,
    CONF_AIR_TEMP_UP_PIN,
    CONF_AIR_TEMP_DOWN_PIN,
    CONF_FILTER_PIN,
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(VentBridge),
        cv.Optional(CONF_UART_PORT, default=1): cv.int_range(min=0, max=2),
        cv.Required(CONF_RX_PIN): pins.internal_gpio_input_pin_number,
        cv.Required(CONF_TX_PIN): pins.internal_gpio_output_pin_number,
        cv.Optional(CONF_BAUD_RATE, default=4800): cv.positive_int,
        **{
            cv.Required(pin): pins.internal_gpio_output_pin_number
            for pin in _BUTTON_PINS
        },
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    add_idf_component(name="vent_core", path=VENT_CORE_PATH)

    # ESPHome pins the IDF compile-time log ceiling at ERROR, which would strip every
    # ESP_LOGI/W/D in vent_core. Raise the ceiling and enable per-tag runtime filtering
    # so only the vent_* tags are verbose; the global default stays at ERROR.
    # A user-supplied sdkconfig_options entry still wins over these.
    add_idf_sdkconfig_option("CONFIG_LOG_MAXIMUM_LEVEL_DEBUG", True)
    add_idf_sdkconfig_option("CONFIG_LOG_DYNAMIC_LEVEL_CONTROL", True)
    add_idf_sdkconfig_option("CONFIG_LOG_TAG_LEVEL_IMPL_NONE", False)
    add_idf_sdkconfig_option("CONFIG_LOG_TAG_LEVEL_IMPL_CACHE_AND_LINKED_LIST", True)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_uart_port(config[CONF_UART_PORT]))
    cg.add(var.set_rx_pin(config[CONF_RX_PIN]))
    cg.add(var.set_tx_pin(config[CONF_TX_PIN]))
    cg.add(var.set_baud_rate(config[CONF_BAUD_RATE]))

    cg.add(var.set_airflow_up_pin(config[CONF_AIRFLOW_UP_PIN]))
    cg.add(var.set_airflow_down_pin(config[CONF_AIRFLOW_DOWN_PIN]))
    cg.add(var.set_air_temp_up_pin(config[CONF_AIR_TEMP_UP_PIN]))
    cg.add(var.set_air_temp_down_pin(config[CONF_AIR_TEMP_DOWN_PIN]))
    cg.add(var.set_filter_pin(config[CONF_FILTER_PIN]))
