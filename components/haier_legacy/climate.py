import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID

AUTO_LOAD = ["climate"]

haier_legacy_ns = cg.esphome_ns.namespace("haier_legacy")
Haier = haier_legacy_ns.class_("Haier", climate.Climate, cg.PollingComponent)

CONFIG_SCHEMA = climate.climate_schema(Haier).extend(
    cv.polling_component_schema("5s")
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
