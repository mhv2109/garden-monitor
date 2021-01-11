#include "../include/apds_3901.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include <math.h>

#define SET_LOW_GAIN(v) v &= ~0x10

#define I2C_MAX_WAIT pdMS_TO_TICKS(13)

static const char *TAG = "APDS 3901";

static esp_err_t set_register(apds_3901 *sensor, uint8_t reg, uint8_t val) {
  i2c_cmd_handle_t cmd;
  esp_err_t err;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_WRITE,
                        I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, val, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT);
  i2c_cmd_link_delete(cmd);

  return err;
}

static esp_err_t get_register(apds_3901 *sensor, uint8_t reg, uint8_t *val) {
  i2c_cmd_handle_t cmd;
  esp_err_t err;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_WRITE,
                        I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_READ,
                        I2C_MASTER_NACK);
  i2c_master_read_byte(cmd, val, I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT);
  i2c_cmd_link_delete(cmd);

  return err;
}

static esp_err_t get_two_registers(apds_3901 *sensor, uint8_t reg,
                                   uint16_t *dat) {
  i2c_cmd_handle_t cmd;
  uint8_t hi = 0, lo = 0;
  esp_err_t err;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_WRITE,
                        I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, 0x20 | reg, I2C_MASTER_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_READ,
                        I2C_MASTER_ACK);
  i2c_master_read_byte(cmd, &lo, I2C_MASTER_ACK);
  i2c_master_read_byte(cmd, &hi, I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT);
  i2c_cmd_link_delete(cmd);

  if (err == ESP_OK) {
    *dat = lo | (hi << 8);
  }

  return err;
}

static esp_err_t apds_3901_poweron(apds_3901 *sensor) {
  esp_err_t err;
  if ((err = set_register(sensor, APDS_3901_CONTROL_REG, APDS_3901_POW_ON)) !=
      ESP_OK)
    ESP_LOGE(TAG, "Failed to power on sensor: %s", esp_err_to_name(err));
  return err;
}

static esp_err_t apds_3901_set_long_integ_time(apds_3901 *sensor) {
  esp_err_t err;
  if ((err = set_register(sensor, APDS_3901_TIMING_REG,
                          APDS_3901_INT_TIME_402_MS)) != ESP_OK)
    ESP_LOGE(TAG, "Failed to configure ADC: %s", esp_err_to_name(err));
  return err;
}

static esp_err_t apds_3901_set_low_gain(apds_3901 *sensor) {
  uint8_t val;
  esp_err_t err;

  if ((err = get_register(sensor, APDS_3901_TIMING_REG, &val)) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read ADC config: %s", esp_err_to_name(err));
    return err;
  }

  SET_LOW_GAIN(val);
  if ((err = set_register(sensor, APDS_3901_TIMING_REG, val)) != ESP_OK)
    ESP_LOGE(TAG, "Failed to set gain on ADC: %s", esp_err_to_name(err));

  return err;
}

static esp_err_t apds_3901_get_ch0(apds_3901 *sensor, uint16_t *dat) {
  esp_err_t err;
  if ((err = get_two_registers(sensor, APDS_3901_DATA0LOW_REG, dat)) !=
      ESP_OK) {
    ESP_LOGW(TAG, "Error reading from ch0: %s", esp_err_to_name(err));
    sensor->p_on = false;
  }
  return err;
}

static esp_err_t apds_3901_get_ch1(apds_3901 *sensor, uint16_t *dat) {
  esp_err_t err;
  if ((err = get_two_registers(sensor, APDS_3901_DATA1LOW_REG, dat)) !=
      ESP_OK) {
    ESP_LOGW(TAG, "Error reading from ch1: %s", esp_err_to_name(err));
    sensor->p_on = false;
  }
  return err;
}

/**
 * @brief Initialize APDS 3901 light sensor on a given I2C bus with given address.
 * @note caller is responsible for allocating and freeing of `*sensor` param
 * @param bus I2C bus on which to initialize sensor
 * @param addr 7-bit I2C 'slave' address of sensor
 * @param sensor return-arg pointer to `apds_3901` struct 
 * @return error
 */
esp_err_t apds_3901__init(i2c_port_t bus, uint8_t addr, apds_3901 *sensor) {
  esp_err_t err;

  sensor->bus = bus;
  sensor->addr = addr;
  sensor->p_on = false;

  if ((err = apds_3901_poweron(sensor)) != ESP_OK)
    return err;
  if ((err = apds_3901_set_low_gain(sensor)) != ESP_OK)
    return err;
  if ((err = apds_3901_set_long_integ_time(sensor)) != ESP_OK)
    return err;

  sensor->p_on = true;
  ESP_LOGI(TAG, "Sensor initialized on bus %d with address %02x", sensor->bus,
           sensor->addr);
  return err;
}

/**
 * @brief Read light intensity in lux from APDS 3901. Must initialize
 * `sensor` using `apds_3901__init` prior to calling this
 * function.
 * @param sensor pointer to already-initialized `apds_3901` struct
 * @param float pointer for returning lux value
 * @return error
 */
esp_err_t apds_3901__read_lux(apds_3901 *sensor, float *lux) {
  esp_err_t err;
  uint16_t ch0, ch1;
  float ch0f, ch1f, ratio;

  // handle power failure on sensor (have to turn it back on)
  if (sensor->p_on == false)
    if ((err = apds_3901__init(sensor->bus, sensor->addr, sensor)) != ESP_OK)
      return err;

  if ((err = apds_3901_get_ch0(sensor, &ch0)) != ESP_OK)
    return err;
  if ((err = apds_3901_get_ch1(sensor, &ch1)) != ESP_OK)
    return err;

  ch1f = (float)ch1;
  ch0f = (float)ch0;

  // No error check required for long integration time
  // ESP_ERROR_CHECK((ch0 > 65535) || (ch1 > 65535));

  ratio = ch1f / ch0f;

  // Use default "Low" gain
  ch0f *= 16;
  ch1f *= 16;

  if (ratio <= 0.5)
    *lux = (0.0304 * ch0f) - ((0.062 * ch0f) * pow((ch1f / ch0f), 1.4));
  else if (ratio <= 0.61)
    *lux = (0.0224 * ch0f) - (0.031 * ch1f);
  else if (ratio <= 0.8)
    *lux = (0.0128 * ch0f) - (0.0153 * ch1f);
  else if (ratio <= 1.3)
    *lux = (0.00146 * ch0f) - (0.00112 * ch1f);
  else
    *lux = 0.0;

  return err;
}
