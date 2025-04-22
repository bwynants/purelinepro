import esphome.codegen as cg
from esphome.components import fan, output
from esphome.components.fan import validate_preset_modes
import esphome.config_validation as cv

from esphome.const import (
    CONF_PRESET_MODES,
    CONF_OUTPUT_ID,
    CONF_RESTORE_VALUE,
    CONF_INITIAL_VALUE

)


from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

ExtractorFan = purelinepro_ns.class_("ExtractorFan", fan.Fan, cg.Component)

CONFIG_SCHEMA = cv.All(
    fan.FAN_SCHEMA.extend(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(ExtractorFan),
            cv.Optional(CONF_PRESET_MODES): validate_preset_modes,
            cv.Optional(CONF_INITIAL_VALUE): cv.float_,
            cv.Optional(CONF_RESTORE_VALUE): cv.boolean,
        }
    ),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    comp = await cg.register_component(var, config)
    await fan.register_fan(var, config)

    hub = await cg.get_variable(config[CONF_PurelinePro_ID])
    cg.add(hub.set_fan(comp))

    if CONF_PRESET_MODES in config:
        cg.add(var.set_preset_modes(config[CONF_PRESET_MODES]))
