import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL,
    DEVICE_CLASS_DURATION,
    UNIT_EMPTY,
    UNIT_PERCENT,
    UNIT_SECOND,
    CONF_ID,
    ICON_PERCENT,
    ICON_TIMER,
    ICON_EMPTY,
)

from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

CONF_TIMER="timer"

# Generate schema for 8 persons
MEASUREMENTS = cv.Schema({

    });

MEASUREMENTS = MEASUREMENTS.extend(
    cv.Schema(
    {
        cv.Optional(CONF_TIMER): sensor.sensor_schema(
            unit_of_measurement=UNIT_SECOND,
            icon=ICON_TIMER,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DURATION,
            state_class=STATE_CLASS_TOTAL,
        ),
    }
    )
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
        }
    )
    .extend(MEASUREMENTS)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_PurelinePro_ID])

    if CONF_TIMER in config:
        sens = await sensor.new_sensor(config[CONF_TIMER])
        cg.add(hub.set_timer(sens))
