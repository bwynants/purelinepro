import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv

from esphome.const import (
    CONF_POWER,
    ENTITY_CATEGORY_CONFIG,
    ICON_POWER,
    ICON_LIGHTBULB,
    ENTITY_CATEGORY_NONE,
    ICON_AIR_FILTER,
)

from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

ExtractorButton = purelinepro_ns.class_("ExtractorButton", button.Button, cg.Component)

CONF_DELAYEDOFF = "delayed_off"
CONF_RESETGREASE = "reset_grease"
CONF_DEFAULTLIGHT = "set_default_light"
CONF_DEFAULTSPEED = "set_default_speed"

TYPES = [
    CONF_POWER,
    CONF_DELAYEDOFF,
    CONF_DEFAULTLIGHT,
    CONF_DEFAULTSPEED,
    CONF_RESETGREASE,
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
            cv.Optional(CONF_POWER): button.button_schema(
                ExtractorButton, entity_category=ENTITY_CATEGORY_NONE, icon=ICON_POWER,
            ),
            cv.Optional(CONF_DELAYEDOFF): button.button_schema(
                ExtractorButton, entity_category=ENTITY_CATEGORY_NONE, icon=ICON_POWER,
            ),
            cv.Optional(CONF_RESETGREASE): button.button_schema(
                ExtractorButton, entity_category=ENTITY_CATEGORY_NONE, icon=ICON_AIR_FILTER,
            ),
            cv.Optional(CONF_DEFAULTLIGHT): button.button_schema(
                ExtractorButton, entity_category=ENTITY_CATEGORY_CONFIG, icon=ICON_LIGHTBULB,
            ),
            cv.Optional(CONF_DEFAULTSPEED): button.button_schema(
                ExtractorButton, entity_category=ENTITY_CATEGORY_CONFIG, icon=ICON_LIGHTBULB,
            ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)

async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await button.new_button(sensor_config)
        cg.add(getattr(hub, f"set_{key}_button")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PurelinePro_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)


