import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import gpio
from esphome.const import CONF_ID

CODEOWNERS = []
AUTO_LOAD = []

shift595_ns = cg.namespace('shift595_input')
Shift595Input = shift595_ns.class_('Shift595Input', cg.Component)

# Pin configuration constants
CONF_DS_PIN = "ds_pin"
CONF_SRC_PIN = "src_pin"
CONF_RCK_PIN = "rck_pin"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Shift595Input),
    cv.Required(CONF_DS_PIN): gpio.GPIO_PIN_SCHEMA,
    cv.Required(CONF_SRC_PIN): gpio.GPIO_PIN_SCHEMA,
    cv.Required(CONF_RCK_PIN): gpio.GPIO_PIN_SCHEMA,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Get pin values from config and assign to the component
    ds_pin = await cg.gpio_pin_expression(config[CONF_DS_PIN])
    src_pin = await cg.gpio_pin_expression(config[CONF_SRC_PIN])
    rck_pin = await cg.gpio_pin_expression(config[CONF_RCK_PIN])

    cg.add(var.set_pins(ds_pin, src_pin, rck_pin))
