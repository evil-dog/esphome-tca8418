import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_KEY
from .. import TCA8418Component, tca8418_ns, CONF_TCA8418_ID

DEPENDENCIES = ["tca8418"]

TCA8418ComponentBinarySensor = tca8418_ns.class_(
  "TCA8418ComponentBinarySensor", binary_sensor.BinarySensor
)

def check_key(obj):
  # do checks to validate CONF_KEY
  # raise cv.Invalid("error_msg")  when CONF_KEY is invalid
  # if key number is >=1 <= 80 it is valid.
  return obj

CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(TCA8418ComponentBinarySensor).extend(
        {
            cv.GenerateID(CONF_TCA8418_ID): cv.use_id(TCA8418Component),
            cv.Required(CONF_KEY): cv.int_
        }
    ),
    check_key,
)

async def to_code(config):
  var = cg.new_Pvariable(config[CONF_ID], config[CONF_KEY])
  await binary_sensor.register_binary_sensor(var, config)
  tca8418 = await cg.get_variable(config[CONF_TCA8418_ID])
  cg.add(tca8418.register_listener(var))
