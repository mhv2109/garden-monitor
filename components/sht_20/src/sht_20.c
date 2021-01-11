#include "../include/sht_20.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#define RH_READ_WAIT_MS 30 // datasheet specifies max 29ms to get a reading
#define T_READ_WAIT_MS 86  // datasheet specifies max 85ms to get a reading

#define I2C_MAX_WAIT pdMS_TO_TICKS(13) // i2c timeout is 13ms
#define MAX_RETRIES 10

static const char *TAG = "SHT 20";

static esp_err_t get_register(sht_20 *sensor, uint8_t reg, uint8_t *val) {
  i2c_cmd_handle_t cmd;
  esp_err_t err;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (SHT_20_I2C_ADDR << 1) | I2C_MASTER_WRITE,
                        I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (SHT_20_I2C_ADDR << 1) | I2C_MASTER_READ,
                        I2C_MASTER_ACK);
  i2c_master_read_byte(cmd, val, I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT);
  i2c_cmd_link_delete(cmd);

  return err;
}

static esp_err_t set_register(sht_20 *sensor, uint8_t reg, uint8_t val) {
  i2c_cmd_handle_t cmd;
  esp_err_t err;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (SHT_20_I2C_ADDR << 1) | I2C_MASTER_WRITE,
                        I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, val, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT);
  i2c_cmd_link_delete(cmd);

  return err;
}

static uint8_t check_crc(uint16_t dat, uint8_t checksum) {
  uint32_t remainder = ((uint32_t)dat << 8), divisor = 0x988000;

  remainder |= checksum;
  for (int i = 0; i < 16; i++) {
    if (remainder & ((uint32_t)1 << (23 - i)))
      remainder ^= divisor;
    divisor >>= 1;
  }

  return (uint8_t)remainder;
}

static esp_err_t get_wide_register(sht_20 *sensor, uint8_t reg, uint16_t *dat, uint32_t delay) {
  i2c_cmd_handle_t cmd;
  uint8_t hi, lo, checksum, att = 0;
  esp_err_t err;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (SHT_20_I2C_ADDR << 1) | I2C_MASTER_WRITE,
                        I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT);
  i2c_cmd_link_delete(cmd);
  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Error requesting sensor reading: %s", esp_err_to_name(err));
    return err;
  }

  // wait for sensor reading
  vTaskDelay(pdMS_TO_TICKS(delay));

  // poll for reading, attempt up to 10 times
  while (att < MAX_RETRIES) {
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHT_20_I2C_ADDR << 1) | I2C_MASTER_READ,
                          I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &hi, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &lo, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &checksum, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT);
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
      *dat = lo | (hi << 8);
      if (check_crc(*dat, checksum) != 0) {
        ESP_LOGD(TAG, "Bad data checksum from sensor, retrying...");
        err = ESP_FAIL;
      } else {
        *dat = (*dat & 0xfffc); // clear temp/humd bits
        break;
      }
    } else {
      ESP_LOGD(TAG, "Sensor read failed, retrying...");
    }
    att++;
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  return err;
}

static esp_err_t set_resolution(sht_20 *sensor) {
  esp_err_t err;
  uint8_t val = 0;

  if ((err = get_register(sensor, SHT_20_READ_USER_REG, &val)) != ESP_OK)
    return err;

  val &= SHT_20_USER_REGISTER_RESOLUTION_RH12_TEMP14;
  return set_register(sensor, SHT_20_WRITE_USER_REG, val);
}

/**
 * @brief Calculate temperature in degrees Celsius.
 * @note Must initialize `*sensor` with `sht_20__init` first!
 * @param sensor pointer to initialized `sht_20` struct
 * @param return-arg pointer to temperature in degrees Celsius
 */
esp_err_t sht_20__read_t(sht_20 *sensor, float *temp) {
  esp_err_t err;
  uint16_t dat = 0;

  if (!(sensor->init))
    if ((err = sht_20__init(sensor->bus, sensor)) != ESP_OK)
      return err;

  if ((err = get_wide_register(sensor, SHT_20_TEMP_MEASURE_NOHOLD, &dat, T_READ_WAIT_MS)) != ESP_OK) {
    sensor->init = false;
    return err;
  }

  // calc T
  *temp = -46.85 + (dat * (175.72 / 65536.0));

  return err;
}

/**
 * @brief Calculate relative humidity.
 * @note Must initialize `*sensor` with `sht_20__init` first!
 * @param sensor pointer to initialized `sht_20` struct
 * @param return-arg pointer to humidity value in percentage
 */
esp_err_t sht_20__read_rh(sht_20 *sensor, float *humd) {
  esp_err_t err;
  uint16_t dat = 0;

  if (!(sensor->init))
    if ((err = sht_20__init(sensor->bus, sensor)) != ESP_OK)
      return err;

  if ((err = get_wide_register(sensor, SHT_20_HUMD_MEASURE_NOHOLD, &dat, RH_READ_WAIT_MS)) != ESP_OK) {
    sensor->init = false;
    return err;
  }

  // calc RH
  *humd = -6.0 + (dat * (125.0 / 65536.0));

  return err;
}

/**
 * @brief Initialize SHT 20 temperature and humidity sensor on a given I2C bus.
 * @note caller is responsible for allocating and freeing of `*sensor` param
 * @param bus I2C bus on which to initialize sensor
 * @param sensor return-arg pointer to `sht_20` struct
 * @return error
 */
esp_err_t sht_20__init(i2c_port_t bus, sht_20 *sensor) {
  esp_err_t err;

  sensor->bus = bus;
  sensor->init = false;

  if ((err = set_resolution(sensor)) != ESP_OK)
    return err;

  sensor->init = true;
  ESP_LOGI(TAG, "Sensor initialized on bus %d with address %02x", sensor->bus,
           SHT_20_I2C_ADDR);
  return err;
}
