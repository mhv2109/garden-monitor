#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/i2c_types.h"
#include <stdio.h>

#include "apds_3901.h"
#include "i2c.h"
#include "seesaw_soil.h"
#include "sht_20.h"
#include "wifi.h"
#include "mqtt.h"

#define I2C_0_SDA_PIN GPIO_NUM_15
#define I2C_0_SCL_PIN GPIO_NUM_2

/// Configurable I2C addresses
#define APDS_3901_I2C_ADDR 0x39
#define SEESAW_I2C_ADDR 0x36

static const char *TAG = "ESP32 Garden Monitor";

void read_sensors(void) {
  esp_err_t err;

  /* Init sensors and read forever */
  if ((err = init_i2c_master(I2C_NUM_0, I2C_0_SDA_PIN, I2C_0_SCL_PIN)) !=
      ESP_OK) {
    ESP_LOGE(TAG, "Error initializing I2C bus: %s", esp_err_to_name(err));
    vTaskDelay(pdMS_TO_TICKS(5000));
    return;
  }

  if ((err = init_apds_3901(I2C_NUM_0, APDS_3901_I2C_ADDR)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing lux sensor: %s", esp_err_to_name(err));
    // don't return here, sensor can be re-initialized on read
  }

  if ((err = init_sht_20(I2C_NUM_0)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing Temp/Humd sensor: %s",
             esp_err_to_name(err));
    // don't return here, sensor can be re-initialized on read
  }

  if ((err = init_soil_sensor(I2C_NUM_0, SEESAW_I2C_ADDR)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing soil sensor: %s", esp_err_to_name(err));
    // don't return here, sensor can be re-initialized on read
  }

  // read and publish sensor readings continuously
  mqtt_publish_all();
}

void app_main(void) {
  esp_chip_info_t chip_info;

  /* Print chip information */
  esp_chip_info(&chip_info);
  printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores,
         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  printf("silicon revision %d, ", chip_info.revision);

  printf("%uMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                       : "external");

  init_wifi();
  read_sensors();
}
