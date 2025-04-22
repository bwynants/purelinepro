import esphome.codegen as cg
from esphome.components import light, output
import esphome.config_validation as cv

from esphome.const import (
    CONF_OUTPUT,
    CONF_OUTPUT_ID,
)

from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

ExtractorLight = purelinepro_ns.class_("ExtractorLight", light.LightOutput, cg.Component)

CONFIG_SCHEMA = cv.All(
    light.BINARY_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(ExtractorLight),
        }
    ),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    comp = await cg.register_component(var, config)
    await light.register_light(var, config)

    hub = await cg.get_variable(config[CONF_PurelinePro_ID])
    cg.add(hub.set_light(comp))
