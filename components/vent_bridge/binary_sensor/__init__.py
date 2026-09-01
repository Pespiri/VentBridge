import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ICON, CONF_TYPE

from .. import CONF_VENT_BRIDGE_ID, VentBridge, vent_bridge_ns

DEPENDENCIES = ["vent_bridge"]

VentBinarySensor = vent_bridge_ns.class_(
    "VentBinarySensor", binary_sensor.BinarySensor, cg.Component
)
VentBinarySensorType = vent_bridge_ns.enum("VentBinarySensorType")

TYPES = {
    "summer_operation": VentBinarySensorType.VENT_BINARY_SUMMER_OPERATION,
    "heater_battery": VentBinarySensorType.VENT_BINARY_HEATER_BATTERY,
    "filter_change": VentBinarySensorType.VENT_BINARY_FILTER_CHANGE,
    "online": VentBinarySensorType.VENT_BINARY_ONLINE,
    "filter_reset": VentBinarySensorType.VENT_BINARY_FILTER_RESET,
}

ICONS = {
    "summer_operation": "mdi:weather-sunny",
    "heater_battery": "mdi:heating-coil",
    "filter_change": "mdi:air-filter",
}


def _default_icon(config):
    icon = ICONS.get(config[CONF_TYPE])
    if icon is not None and CONF_ICON not in config:
        config = config.copy()
        config[CONF_ICON] = icon
    return config


CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(VentBinarySensor)
    .extend(
        {
            cv.GenerateID(CONF_VENT_BRIDGE_ID): cv.use_id(VentBridge),
            cv.Required(CONF_TYPE): cv.enum(TYPES, lower=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    _default_icon,
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config, config[CONF_TYPE])
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_VENT_BRIDGE_ID])
    cg.add(var.set_parent(parent))
