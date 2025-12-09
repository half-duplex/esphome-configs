// Honeywell HSC I2C Sensors
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace honeywell_hsc {

class HoneywellHSCSensor : public PollingComponent, public i2c::I2CDevice {
 public:
  void dump_config() override;
  void update() override;

  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { this->temperature_sensor_ = temperature_sensor; }
  void set_pressure_sensor(sensor::Sensor *pressure_sensor) { this->pressure_sensor_ = pressure_sensor; }

  void set_min_pressure(float _min_pressure) { this->min_pressure = _min_pressure; };
  void set_max_pressure(float _max_pressure) { this->max_pressure = _max_pressure; };

 protected:
  float min_pressure = 0.0;
  float max_pressure = 0.0;

  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *pressure_sensor_{nullptr};

  uint8_t raw_data_[4];        // I2C Read Buffer
  uint8_t status_ = 0;         // byte to hold status information.
  int pressure_count_ = 0;     // hold raw pressure data (14 - bit, 0 - 16384)
  int temperature_count_ = 0;  // hold raw temperature data (11 - bit, 0 - 2048)

  int rawpressure_();
  int rawtemperature_();
  float countstopressure_(int counts, float min_pressure, float max_pressure);
  float countstotemperatures_(int counts);
  float read_pressure_();
  float read_temperature_();
  uint8_t readsensor_();
};

}  // namespace honeywell_hsc
}  // namespace esphome
