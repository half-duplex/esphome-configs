import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_PRESSURE,
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS
)

DEPENDENCIES = ["i2c"]

CONF_MIN_PRESSURE = "min_pressure"
CONF_MAX_PRESSURE = "max_pressure"

honeywell_hsc_ns = cg.esphome_ns.namespace("honeywell_hsc")
HONEYWELLHSCSensor = honeywell_hsc_ns.class_(
    "HoneywellHSCSensor", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(HONEYWELLHSCSensor),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PRESSURE): sensor.sensor_schema(
                unit_of_measurement="psi",
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ).extend(
                {
                    cv.Required(CONF_MIN_PRESSURE): cv.float_,
                    cv.Required(CONF_MAX_PRESSURE): cv.float_,
                }
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x68))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if temperature_config := config.get(CONF_TEMPERATURE):
        sens = await sensor.new_sensor(temperature_config)
        cg.add(var.set_temperature_sensor(sens))

    if pressure_config := config.get(CONF_PRESSURE):
        sens = await sensor.new_sensor(pressure_config)
        cg.add(var.set_pressure_sensor(sens))
        cg.add(var.set_min_pressure(pressure_config[CONF_MIN_PRESSURE]))
        cg.add(var.set_max_pressure(pressure_config[CONF_MAX_PRESSURE]))
