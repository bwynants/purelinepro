import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from esphome.const import (
    DEVICE_CLASS_EMPTY,
)

from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

CONF_CleanGrease="cleangrease"

TYPES = [
    CONF_CleanGrease,
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
            cv.Optional(
                CONF_CleanGrease
            ): binary_sensor.binary_sensor_schema(),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)

async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await binary_sensor.new_binary_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_binary_sensor")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PurelinePro_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)

