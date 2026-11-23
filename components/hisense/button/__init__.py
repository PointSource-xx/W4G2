import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import ENTITY_CATEGORY_CONFIG

from ..climate import CONF_HISENSE_ID, hisense_ac_ns, AirConditioner

AirConditionFilterResetButton = hisense_ac_ns.class_(
    "AirConditionFilterResetButton",
    button.Button,
)

CONF_FILTER_RESET = "filter_reset"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_HISENSE_ID): cv.use_id(AirConditioner),
        # Clears the unit's filter cleaning reminder (control byte 37).
        cv.Optional(CONF_FILTER_RESET): button.button_schema(
            AirConditionFilterResetButton,
            icon="mdi:air-filter",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_HISENSE_ID])
    if conf := config.get(CONF_FILTER_RESET):
        var = await button.new_button(conf)
        cg.add(var.set_parent(paren))
