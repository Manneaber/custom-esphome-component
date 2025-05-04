import esphome.codegen as cg
from esphome.components import climate
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import automation
from esphome.core import coroutine

AUTO_LOAD = ['climate']

carrier42aaf_ns = cg.esphome_ns.namespace("carrier42aaf2")
Carrier42AAF = carrier42aaf_ns.class_("Carrier42AAF2", climate.Climate, cg.Component)

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(Carrier42AAF),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    cg.add_library("IRremoteESP8266", "2.8.6")
    cg.add_library("ESP_EEPROM", "2.2.1")
