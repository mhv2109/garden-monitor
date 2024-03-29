#include "../include/seesaw_soil.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/projdefs.h"

// Config constants
#define SEESAW_DELAY_MS 1000
#define I2C_MAX_WAIT pdMS_TO_TICKS(13)
static const char *TAG = "Adafruit Seesaw soil sensor";

/// Representation of sensor
typedef struct seesaw_soil {
  i2c_port_t bus;
  uint8_t addr;
} seesaw_soil_t;

/// Global vars
static seesaw_soil_t *SENSOR = NULL;

static esp_err_t get_wide_register(seesaw_soil_t *sensor, uint8_t reg_h,
                                   uint8_t reg_l, uint16_t *dat,
                                   uint32_t delay) {
  i2c_cmd_handle_t cmd;
  uint8_t hi = 0xff, lo = 0xff;
  esp_err_t err = ESP_OK;

  // initialize return var
  *dat = 65535;

  // request sensor touch sensor read
  // this thing is really flaky, just retry until it works
  for (;;) {
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->addr << 1) | I2C_MASTER_WRITE,
                          I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, reg_h, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, reg_l, I2C_MASTER_ACK);
    i2c_master_stop(cmd);

    if ((err = i2c_master_cmd_begin(sensor->bus, cmd, I2C_MAX_WAIT)) ==
        ESP_OK) {
      i2c_cmd_link_delete(cmd);
      break;
    }
    ESP_LOGD(TAG, "Error requesting sensor reading: %s, retrying...",
             esp_err_to_name(err));
    i2c_cmd_link_delete(cmd);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // wait for sensor reading
  vTaskDelay(pdMS_TO_TICKS(delay));

  // this thing is really flaky, just retry until it works
  for (;;) {
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
        ESP_LOGD(TAG, "Invalid data from sensor, retrying...");
        err = ESP_FAIL;
      } else {
        i2c_cmd_link_delete(cmd);
        break;
      }
    } else {
      ESP_LOGD(TAG, "Error reading from sensor: %s, retrying...",
               esp_err_to_name(err));
    }

    i2c_cmd_link_delete(cmd);
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error reading data from sensor: %s", esp_err_to_name(err));
  }

  return err;
}

/**
 * @brief Read Soil moisture.
 * @note must initialize sensor with `init_soil_sensor`
 * @note moisture readings take 1s to complete
 * @note blocks until a read is successful
 * @param moist return-arg value for moisture in range 0 (very dry) to 1023
 * (very wet)
 * @return error
 */
esp_err_t read_soil_moisture(uint16_t *moist) {
  if (SENSOR == NULL) {
    ESP_LOGE(TAG, "Sensor not initialized");
    return ESP_FAIL;
  }

  return get_wide_register(SENSOR, SEESAW_TOUCH_BASE,
                           SEESAW_TOUCH_CHANNEL_OFFSET + SEESAW_TOUCH_PIN,
                           moist, SEESAW_DELAY_MS);
}

/**
 * @brief Initialize a Adafruit STEMMA soil sensor build on their Seesaw
 * platform.
 * @param bus I2C bus on which to initialize sensor
 * @param addr I2C address for soil sensor
 * @return error
 */
esp_err_t init_soil_sensor(i2c_port_t bus, uint8_t addr) {
  esp_err_t err;

  if (SENSOR != NULL) {
    if (SENSOR->bus != bus || SENSOR->addr != addr) {
      ESP_LOGW(TAG, "Sensor already initialized with different params");
      err = ESP_FAIL;
    } else {
      ESP_LOGI(TAG, "Sensor already initialized");
      err = ESP_OK;
    }
    return err;
  }

  SENSOR = (seesaw_soil_t *)malloc(sizeof(seesaw_soil_t));
  SENSOR->bus = bus;
  SENSOR->addr = addr;

  return ESP_OK;
}
