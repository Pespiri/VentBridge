import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ICON, CONF_TYPE

from .. import CONF_VENT_BRIDGE_ID, VentBridge, vent_bridge_ns

DEPENDENCIES = ["vent_bridge"]

VentSelect = vent_bridge_ns.class_("VentSelect", select.Select, cg.Component)
VentSelectType = vent_bridge_ns.enum("VentSelectType")

TYPES = {
    "airflow": VentSelectType.VENT_SELECT_AIRFLOW,
    "air_temp": VentSelectType.VENT_SELECT_AIR_TEMP,
}

# Order must mirror the level enums; the C++ side maps option index to level,
# offset by the first selectable level (AIRFLOW_LEVEL_MIN / AIR_TEMP_LEVEL_NONE).
OPTIONS = {
    "airflow": ["min", "norm", "max"],
    "air_temp": ["none", "low", "low/med", "med", "med/high", "high"],
}

ICONS = {
    "airflow": "mdi:fan",
    "air_temp": "mdi:thermometer",
}


def _default_icon(config):
    icon = ICONS.get(config[CONF_TYPE])
    if icon is not None and CONF_ICON not in config:
        config = config.copy()
        config[CONF_ICON] = icon
    return config


CONFIG_SCHEMA = cv.All(
    select.select_schema(VentSelect)
    .extend(
        {
            cv.GenerateID(CONF_VENT_BRIDGE_ID): cv.use_id(VentBridge),
            cv.Required(CONF_TYPE): cv.one_of(*OPTIONS, lower=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    _default_icon,
)


async def to_code(config):
    kind = config[CONF_TYPE]
    var = await select.new_select(config, TYPES[kind], options=OPTIONS[kind])
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_VENT_BRIDGE_ID])
    cg.add(var.set_parent(parent))
