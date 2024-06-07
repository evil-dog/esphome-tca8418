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
  # Valid key numbers are >=97 and <=114
  if CONF_KEY in obj:
    if obj[CONF_KEY] < 97 or obj[CONF_KEY] > 114:
      raise cv.Invalid("The key code must be between 97 and 114")
  else:
    raise cv.Invalid("Missing key code.")
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
