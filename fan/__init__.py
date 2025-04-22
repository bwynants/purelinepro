import esphome.codegen as cg
from esphome.components import fan, output
import esphome.config_validation as cv

from esphome.const import (
    CONF_OUTPUT,
    CONF_OUTPUT_ID,
)


from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

ExtractorFan = purelinepro_ns.class_("ExtractorFan", fan.Fan, cg.Component)

CONFIG_SCHEMA = cv.All(
    fan.FAN_SCHEMA.extend(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(ExtractorFan),
        }
    ),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    comp = await cg.register_component(var, config)
    await fan.register_fan(var, config)

    hub = await cg.get_variable(config[CONF_PurelinePro_ID])
    cg.add(hub.set_fan(comp))
