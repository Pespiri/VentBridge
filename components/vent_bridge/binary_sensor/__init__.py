import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_TYPE

from .. import CONF_VENT_BRIDGE_ID, VentBridge, vent_bridge_ns

DEPENDENCIES = ["vent_bridge"]

VentBinarySensor = vent_bridge_ns.class_(
    "VentBinarySensor", binary_sensor.BinarySensor, cg.Component
)
VentBinarySensorType = vent_bridge_ns.enum("VentBinarySensorType")

TYPES = {
    "summer": VentBinarySensorType.VENT_BINARY_SUMMER,
    "filter": VentBinarySensorType.VENT_BINARY_FILTER,
    "online": VentBinarySensorType.VENT_BINARY_ONLINE,
}

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(VentBinarySensor)
    .extend(
        {
            cv.GenerateID(CONF_VENT_BRIDGE_ID): cv.use_id(VentBridge),
            cv.Required(CONF_TYPE): cv.enum(TYPES, lower=True),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config, config[CONF_TYPE])
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_VENT_BRIDGE_ID])
    cg.add(var.set_parent(parent))
