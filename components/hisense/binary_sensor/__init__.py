import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_PROBLEM, ENTITY_CATEGORY_DIAGNOSTIC

from ..climate import CONF_HISENSE_ID, AirConditioner

CONF_REMOTE_USED = "remote_used"
CONF_FILTER = "filter"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_HISENSE_ID): cv.use_id(AirConditioner),
        # On when the unit was controlled with its IR remote or buttons since the
        # previous command (status byte 36 bit 3).
        cv.Optional(CONF_REMOTE_USED): binary_sensor.binary_sensor_schema(
            icon="mdi:remote",
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        # On when the unit asks for the indoor filter to be cleaned (status byte 35
        # bit 3). Cleared by the filter_reset button.
        cv.Optional(CONF_FILTER): binary_sensor.binary_sensor_schema(
            icon="mdi:air-filter",
            device_class=DEVICE_CLASS_PROBLEM,
        ),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_HISENSE_ID])
    if conf := config.get(CONF_REMOTE_USED):
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(paren.set_remote_used_binary_sensor(sens))
    if conf := config.get(CONF_FILTER):
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(paren.set_filter_binary_sensor(sens))
