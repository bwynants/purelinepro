import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from esphome.const import (
    STATE_CLASS_TOTAL,
    DEVICE_CLASS_DURATION,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_SECOND,
    UNIT_MINUTE,
    ICON_TIMER,
)

from .. import PurelinePro, purelinepro_ns, CONF_PurelinePro_ID

CONF_TIMER="timer"
CONF_GREASETIMER="greasetimer"
CONF_OPERATINGHOURSLED="operating_hours_led"
CONF_OPERATINGHOURSFAN="operating_hours_fan"

TYPES = [
    CONF_TIMER,
    CONF_GREASETIMER,
    CONF_OPERATINGHOURSLED,
    CONF_OPERATINGHOURSFAN,
]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_PurelinePro_ID): cv.use_id(PurelinePro),
            cv.Optional(CONF_TIMER): sensor.sensor_schema(  
                            unit_of_measurement=UNIT_SECOND,
                            icon=ICON_TIMER,
                            accuracy_decimals=0,
                            device_class=DEVICE_CLASS_DURATION,
                            state_class=STATE_CLASS_TOTAL,
                ),
            cv.Optional(CONF_GREASETIMER): sensor.sensor_schema(  
                            unit_of_measurement=UNIT_MINUTE,
                            icon=ICON_TIMER,
                            accuracy_decimals=0,
                            device_class=DEVICE_CLASS_DURATION,
                            state_class=STATE_CLASS_TOTAL,
                ),
            cv.Optional(CONF_OPERATINGHOURSLED): sensor.sensor_schema(  
                            unit_of_measurement=UNIT_MINUTE,
                            icon=ICON_TIMER,
                            accuracy_decimals=0,
                            device_class=DEVICE_CLASS_DURATION,
                            state_class=STATE_CLASS_TOTAL_INCREASING,
                ),
            cv.Optional(CONF_OPERATINGHOURSFAN): sensor.sensor_schema(  
                            unit_of_measurement=UNIT_MINUTE,
                            icon=ICON_TIMER,
                            accuracy_decimals=0,
                            device_class=DEVICE_CLASS_DURATION,
                            state_class=STATE_CLASS_TOTAL,
                ),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)

async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        var = await sensor.new_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_sensor")(var))


async def to_code(config):
    hub = await cg.get_variable(config[CONF_PurelinePro_ID])
    for key in TYPES:
        await setup_conf(config, key, hub)
