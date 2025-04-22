import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
    DEVICE_CLASS_SWITCH,
)

CONF_POWERTOGGLE = "powertoggle"

from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

extractorswitch_ns = cg.esphome_ns.namespace("purelinepro")
ExtractorSwitch = extractorswitch_ns.class_("ExtractorSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
    cv.Optional(CONF_POWERTOGGLE): switch.switch_schema(
        ExtractorSwitch,
        device_class=DEVICE_CLASS_SWITCH,
        entity_category=ENTITY_CATEGORY_CONFIG,
    )
    .extend(cv.COMPONENT_SCHEMA),
}


async def to_code(config):
    if c := config.get(CONF_POWERTOGGLE):
        s = await switch.new_switch(c)
        await cg.register_component(s, c)

        hub = await cg.get_variable(config[CONF_PurelinePro_ID])
        cg.add(hub.set_powertoggle_switch(s))

