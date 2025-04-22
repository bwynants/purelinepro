import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    ble_client,
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

DEPENDENCIES = ["esp32", "ble_client", "time"]

MULTI_CONF = True

CONF_PurelinePro_ID = "purelinepro_id"

purelinepro_ns = cg.esphome_ns.namespace("purelinepro")
PurelinePro = purelinepro_ns.class_(
    "PurelinePro", cg.PollingComponent, ble_client.BLEClientNode
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(PurelinePro),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    }
).extend(cv.polling_component_schema("500ms")).extend(ble_client.BLE_CLIENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    if CONF_TIME_ID in config:
        time_ = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_id(time_))

