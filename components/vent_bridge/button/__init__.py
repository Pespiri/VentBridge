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
    "airflow_up": "BUTTON_AIRFLOW_UP",
    "airflow_down": "BUTTON_AIRFLOW_DOWN",
    "air_temp_up": "BUTTON_AIR_TEMP_UP",
    "air_temp_down": "BUTTON_AIR_TEMP_DOWN",
    "filter_long": "BUTTON_FILTER_LONG",
}

ICONS = {
    "airflow_up": "mdi:fan-plus",
    "airflow_down": "mdi:fan-minus",
    "air_temp_up": "mdi:thermometer-plus",
    "air_temp_down": "mdi:thermometer-minus",
    "filter_long": "mdi:air-filter",
}


def _default_icon(config):
    icon = ICONS.get(config[CONF_TYPE])
    if icon is not None and CONF_ICON not in config:
        config = config.copy()
        config[CONF_ICON] = icon
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
