import esphome.codegen as cg
from esphome.components import climate, sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_SENSOR_ID
from esphome import automation
from esphome.core import coroutine

AUTO_LOAD = ['climate']

carrier42aaf_ns = cg.esphome_ns.namespace("carrier42aaf")
Carrier42AAF = carrier42aaf_ns.class_("Carrier42AAF", climate.Climate, cg.Component)

ACTION_BASE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(Carrier42AAF),
    }
)

def templatize(value):
    if isinstance(value, cv.Schema):
        value = value.schema
    ret = {}
    for key, val in value.items():
        ret[key] = cv.templatable(val)
    return cv.Schema(ret)

def register_action(name, type_, schema):
    validator = templatize(schema).extend(ACTION_BASE_SCHEMA)
    registerer = automation.register_action(f"carrier42aaf.{name}", type_, validator)

    def decorator(func):
        async def new_func(config, action_id, template_arg, args):
            ac_ = await cg.get_variable(config[CONF_ID])
            var = cg.new_Pvariable(action_id, template_arg)
            cg.add(var.set_parent(ac_))
            await coroutine(func)(var, config, args)
            return var

        return registerer(new_func)

    return decorator

CONFIG_SCHEMA = climate.climate_schema(Carrier42AAF).extend(
    {
        cv.Optional("temperature_sensor"): cv.use_id(sensor.Sensor),
    }
).extend(cv.COMPONENT_SCHEMA)

# Custom Actions
LightToggleAction = carrier42aaf_ns.class_("LightToggleAction", automation.Action)

# Light Toggle action
@register_action(
    "light_toggle",
    LightToggleAction,
    cv.Schema({}),
)
async def light_toggle_to_code(var, config, args):
    pass


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    
    if "temperature_sensor" in config:
        sens = await cg.get_variable(config["temperature_sensor"])
        cg.add(var.set_current_temperature_sensor(sens))
    
    cg.add_library("IRremoteESP8266", "2.8.6")
    cg.add_library("ESP_EEPROM", "2.2.1")
