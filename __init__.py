import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    time,
)
from esphome.const import (
    CONF_ID,
    CONF_TIME_ID,
)
AUTO_LOAD = [
    "switch", "light", "fan", "sensor"
]

CODEOWNERS = ["@bwynants"]

MULTI_CONF = True

CONF_PurelinePro_ID = "PurelinePro_id"
CONF_TIME_OFFSET = "timeoffset"

purelinepro_ns = cg.esphome_ns.namespace("purelinepro")
PurelinePro = purelinepro_ns.class_(
    "PurelinePro", cg.Component
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(PurelinePro),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_TIME_ID in config:
        time_ = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_id(time_))
    cg.add_library("ESP32 BLE Arduino", "2.0.0")

