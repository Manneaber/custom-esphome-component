import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID

CODEOWNERS = []
AUTO_LOAD = ['gpio']

shift595_ns = cg.esphome_ns.namespace('shift595_input')
Shift595Input = shift595_ns.class_('Shift595Input', cg.Component)

# Pin configuration constants
CONF_DS_PIN = "ds_pin"
CONF_SRC_PIN = "src_pin"
CONF_RCK_PIN = "rck_pin"

# Define schema for our component's pins
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Shift595Input),
    cv.Required(CONF_DS_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_SRC_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_RCK_PIN): pins.gpio_input_pin_schema,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Get pin expressions from the configuration and assign to the component
    ds_pin = await cg.gpio_pin_expression(config[CONF_DS_PIN])
    src_pin = await cg.gpio_pin_expression(config[CONF_SRC_PIN])
    rck_pin = await cg.gpio_pin_expression(config[CONF_RCK_PIN])

    cg.add(var.set_pins(ds_pin, src_pin, rck_pin))
