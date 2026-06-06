import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, uart
from esphome.const import CONF_ID

AUTO_LOAD = ["climate"]
DEPENDENCIES = ["uart"]

haier_legacy_ns = cg.esphome_ns.namespace("haier_legacy")
Haier = haier_legacy_ns.class_(
    "Haier", climate.Climate, cg.PollingComponent, uart.UARTDevice
)

CONFIG_SCHEMA = (
    climate.climate_schema(Haier)
    .extend(cv.polling_component_schema("5s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    await uart.register_uart_device(var, config)
