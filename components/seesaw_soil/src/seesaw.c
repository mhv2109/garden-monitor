#include "../include/seesaw_soil.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
#include "hal/i2c_types.h"

// Config constants
#define SEESAW_DELAY_MS 1000
#define MAX_RETRIES 10
#define I2C_MAX_WAIT pdMS_TO_TICKS(13)

static const char *TAG = "Adafruit Seesaw soil sensor";

static esp_err_t get_wide_register(seesaw_soil *sensor, uint8_t reg_h,
                                   uint8_t reg_l, uint16_t *dat,
                                   uint32_t delay) {
  i2c_cmd_handle_t cmd;
  uint8_t hi = 0xff, lo = 0xff, att = 0;
  esp_err_t err = ESP_OK;

  // initialize return var
  *dat = 65535;

  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_WRITE,
                        I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, reg_h, I2C_MASTER_ACK);
  i2c_master_write_byte(cmd, reg_l, I2C_MASTER_ACK);
  i2c_master_stop(cmd);
  if ((err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT)) != ESP_OK) {
    ESP_LOGE(TAG, "Error requesting sensor reading: %s", esp_err_to_name(err));
    return err;
  }
  i2c_cmd_link_delete(cmd);

  // wait for sensor reading
  vTaskDelay(pdMS_TO_TICKS(delay));

  // poll for reading, attempt up to 10 times
  while (att < MAX_RETRIES) {
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_READ,
                          I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &hi, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &lo, I2C_MASTER_ACK);
    i2c_master_stop(cmd);

    if ((err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT)) ==
        ESP_OK) {
      ESP_LOGD(TAG, "Wide register lo: %02x", lo);
      ESP_LOGD(TAG, "Wide register hi: %02x", hi);
      *dat = lo | (hi << 8);
      if (*dat == 65535) {
        ESP_LOGE(TAG, "Invalid data from sensor, retrying...");
        err = ESP_FAIL;
      } else {
        i2c_cmd_link_delete(cmd);
        break;
      }
    } else {
      ESP_LOGD(TAG, "Sensor read failed, retrying...");
    }
    i2c_cmd_link_delete(cmd);
    att++;
    vTaskDelay(pdMS_TO_TICKS(5));
  }

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error reading data from sensor: %s", esp_err_to_name(err));
  }

  return err;
}

/**
 * @brief Read Soil moisture.
 * @note must initialize `*sensor` param with `seesaw__init`
 * @note moisture readings take 1s to complete
 * @param sensor pointer to initialized `seesaw_soil` struct
 * @param moist return-arg value for moisture in range 0 (very dry) to 1023 (very wet)
 * @return error
 */
esp_err_t seesaw__read_m(seesaw_soil *sensor, uint16_t *moist) {
  return get_wide_register(sensor, SEESAW_TOUCH_BASE,
                           SEESAW_TOUCH_CHANNEL_OFFSET + SEESAW_TOUCH_PIN,
                           moist, SEESAW_DELAY_MS);
}

/**
 * @brief Initialize a Adafruit STEMMA soil sensor build on their Seesaw platform.
 * @param bus I2C bus on which to initialize sensor
 * @param addr I2C address for soil sensor
 * @param sensor return-arg pointer to seesaw_soil struct
 * @return error
 */
esp_err_t seesaw__init(i2c_port_t bus, uint8_t addr, seesaw_soil *sensor) {
  sensor->bus = bus;
  sensor->addr = addr;
  return ESP_OK;
}