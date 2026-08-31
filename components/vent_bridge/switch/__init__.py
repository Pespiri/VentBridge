import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import ENTITY_CATEGORY_CONFIG

from .. import CONF_VENT_BRIDGE_ID, VentBridge, vent_bridge_ns

DEPENDENCIES = ["vent_bridge"]

VentTraceSwitch = vent_bridge_ns.class_("VentTraceSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = (
    switch.switch_schema(
        VentTraceSwitch,
        icon="mdi:bug-outline",
        entity_category=ENTITY_CATEGORY_CONFIG,
        default_restore_mode="DISABLED",
    )
    .extend(
        {
            cv.GenerateID(CONF_VENT_BRIDGE_ID): cv.use_id(VentBridge),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_VENT_BRIDGE_ID])
    cg.add(var.set_parent(parent))
