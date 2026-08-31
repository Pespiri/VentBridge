import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ICON, CONF_TYPE

from .. import CONF_VENT_BRIDGE_ID, VentBridge, vent_bridge_ns

DEPENDENCIES = ["vent_bridge"]

VentButton = vent_bridge_ns.class_("VentButton", button.Button, cg.Component)

# Bare enumerators of vent_button_enum_t; emitted verbatim so the numeric values
# are never duplicated here.
BUTTON_TYPES = {
    "fan_up": "BUTTON_FAN_UP",
    "fan_down": "BUTTON_FAN_DOWN",
    "temp_up": "BUTTON_TEMP_UP",
    "temp_down": "BUTTON_TEMP_DOWN",
    "filter_long": "BUTTON_FILTER_LONG",
}

ICONS = {
    "fan_up": "mdi:fan-plus",
    "fan_down": "mdi:fan-minus",
    "temp_up": "mdi:thermometer-plus",
    "temp_down": "mdi:thermometer-minus",
    "filter_long": "mdi:air-filter",
}


def _default_icon(config):
    if CONF_ICON not in config:
        config = config.copy()
        config[CONF_ICON] = ICONS[config[CONF_TYPE]]
    return config


CONFIG_SCHEMA = cv.All(
    button.button_schema(VentButton)
    .extend(
        {
            cv.GenerateID(CONF_VENT_BRIDGE_ID): cv.use_id(VentBridge),
            cv.Required(CONF_TYPE): cv.one_of(*BUTTON_TYPES, lower=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    _default_icon,
)


async def to_code(config):
    var = await button.new_button(config, cg.RawExpression(BUTTON_TYPES[config[CONF_TYPE]]))
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_VENT_BRIDGE_ID])
    cg.add(var.set_parent(parent))
