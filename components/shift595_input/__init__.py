import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_DS_PIN, CONF_SRC_PIN, CONF_RCK_PIN

CODEOWNERS = []
AUTO_LOAD = []

shift595_ns = cg.namespace('shift595_input')
Shift595Input = shift595_ns.class_('Shift595Input', cg.Component)

CONF_DS_PIN = "ds_pin"
CONF_SRC_PIN = "src_pin"
CONF_RCK_PIN = "rck_pin"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Shift595Input),
    cv.Required(CONF_DS_PIN): cv.gpio_input_pin_schema,
    cv.Required(CONF_SRC_PIN): cv.gpio_input_pin_schema,
    cv.Required(CONF_RCK_PIN): cv.gpio_input_pin_schema,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    ds = await cg.gpio_pin_expression(config[CONF_DS_PIN])
    src = await cg.gpio_pin_expression(config[CONF_SRC_PIN])
    rck = await cg.gpio_pin_expression(config[CONF_RCK_PIN])
    cg.add(var.set_pins(ds, src, rck))
