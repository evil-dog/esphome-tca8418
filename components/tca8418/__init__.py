import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import i2c, key_provider
from esphome.const import CONF_ID, CONF_PIN

AUTO_LOAD = ["key_provider"]

DEPENDENCIES = ["i2c"]
MULTI_CONF = True

tca8418_ns = cg.esphome_ns.namespace("tca8418")
TCA8418Component = tca8418_ns.class_(
    "TCA8418Component",
    key_provider.KeyProvider,
    cg.Component,
    i2c.I2CDevice
)

CONF_TCA8418_ID = "tca8418_id"
CONF_INTERRUPT_PIN = "int_pin"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TCA8418Component),
            cv.Required(CONF_INTERRUPT_PIN): pins.gpio_input_pin_schema
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(None))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cg.register_i2c_device(var, config)

    int_pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
    cg.add(var.set_int_pin(int_pin))
