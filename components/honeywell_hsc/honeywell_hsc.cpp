#include "honeywell_hsc.h"
#include "esphome/core/log.h"

namespace esphome {
namespace honeywell_hsc {

static const char *const TAG = "honeywell_hsc";

const float MIN_COUNT = 1638.4;   // 1638 counts (10% of 2^14 counts or 0x0666)
const float MAX_COUNT = 14745.6;  // 14745 counts (90% of 2^14 counts or 0x3999)

uint8_t HoneywellHSCSensor::readsensor_() {
  if (this->read(raw_data_, 4) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
    this->status_set_warning(LOG_STR("couldn't read sensor data"));
    return 0x03;
  }

  // Check the status codes:
  // status = 0 : normal operation
  // status = 1 : device in command mode
  // status = 2 : stale data
  // status = 3 : diagnostic condition
  status_ = raw_data_[0] >> 6 & 0x3;
  ESP_LOGV(TAG, "Sensor status %d", status_);

  // if device is normal and there is new data, bitmask and save the raw data
  if (status_ == 0) {
    // 14 - bit pressure is the last 6 bits of byte 0 (high bits) & all of byte 1 (lowest 8 bits)
    pressure_count_ = ((uint16_t) (raw_data_[0]) << 8 & 0x3F00) | ((uint16_t) (raw_data_[1]) & 0xFF);
    // 11 - bit temperature is all of byte 2 (lowest 8 bits) and the first three bits of byte 3
    temperature_count_ = (((uint16_t) (raw_data_[2]) << 3) & 0x7F8) | (((uint16_t) (raw_data_[3]) >> 5) & 0x7);
    ESP_LOGV(TAG, "Sensor pressure_count_ %d", pressure_count_);
    ESP_LOGV(TAG, "Sensor temperature_count_ %d", temperature_count_);
    this->status_clear_warning();
  } else if (status_ == 0x03) {
    this->status_set_warning(LOG_STR("diagnostic fault 0x03"));
  }
  
  return status_;
}

// The pressure value from the most recent reading in raw counts
int HoneywellHSCSensor::rawpressure_() { return pressure_count_; }

// The temperature value from the most recent reading in raw counts
int HoneywellHSCSensor::rawtemperature_() { return temperature_count_; }

// Converts a digital pressure measurement in counts to pressure measured
float HoneywellHSCSensor::countstopressure_(const int counts, const float _min_pressure, const float _max_pressure) {
  return ((((float) counts - MIN_COUNT) * (_max_pressure - _min_pressure)) / (MAX_COUNT - MIN_COUNT)) + _min_pressure;
}

// Converts a digital temperature measurement in counts to temperature in C
// This will be invalid if sensore daoes not have temperature measurement capability
float HoneywellHSCSensor::countstotemperatures_(const int counts) { return (((float) counts / 2047.0) * 200.0) - 50.0; }

// Pressure value from the most recent reading in units
float HoneywellHSCSensor::read_pressure_() {
  return countstopressure_(pressure_count_, min_pressure, max_pressure);
}

// Temperature value from the most recent reading in degrees C
float HoneywellHSCSensor::read_temperature_() { return countstotemperatures_(temperature_count_); }

void HoneywellHSCSensor::update() {
  ESP_LOGV(TAG, "Update Honeywell HSC Sensor");
  if (readsensor_() == 0) {
    if (this->pressure_sensor_ != nullptr)
      this->pressure_sensor_->publish_state(read_pressure_() * 1.0);
    if (this->temperature_sensor_ != nullptr)
      this->temperature_sensor_->publish_state(read_temperature_() * 1.0);
  }
}

void HoneywellHSCSensor::dump_config() {
  //  LOG_SENSOR("", "HONEYWELLHSC", this);
  ESP_LOGCONFIG(TAG,
                "  Min Pressure Range: %0.1f\n"
                "  Max Pressure Range: %0.1f",
                min_pressure, max_pressure);
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace honeywell_hsc
}  // namespace esphome
