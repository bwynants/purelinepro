import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from esphome.const import (
    DEVICE_CLASS_SWITCH,
    ENTITY_CATEGORY_CONFIG,
    ICON_FAN,
    ICON_POWER
)

from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

CONF_RECIRCULATE="recirculate"
CONF_ENABLED="enabled"

TYPES = [
    CONF_RECIRCULATE,
    CONF_ENABLED,
]

extractorswitch_ns = cg.esphome_ns.namespace("purelinepro")
ExtractorSwitch = extractorswitch_ns.class_("ExtractorSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
            cv.Optional(
                CONF_RECIRCULATE
            ): switch.switch_schema(ExtractorSwitch,
                        device_class=DEVICE_CLASS_SWITCH,
                        entity_category=ENTITY_CATEGORY_CONFIG,
                        icon=ICON_FAN),
            cv.Optional(
                CONF_ENABLED
            ): switch.switch_schema(ExtractorSwitch,
                        device_class=DEVICE_CLASS_SWITCH,
                        entity_category=ENTITY_CATEGORY_CONFIG,
                        icon=ICON_POWER),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)

async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await switch.new_switch(sensor_config)
        cg.add(getattr(hub, f"set_{key}_switch")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PurelinePro_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)

