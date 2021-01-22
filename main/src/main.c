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
#include "nvs.h"
#include "seesaw_soil.h"
#include "sht_20.h"
#include "smartconfig.h"
#include "wifi.h"

#define I2C_0_SDA_PIN GPIO_NUM_15
#define I2C_0_SCL_PIN GPIO_NUM_2

/// Configurable I2C addresses
#define APDS_3901_I2C_ADDR 0x39
#define SEESAW_I2C_ADDR 0x36

static const char *TAG = "ESP32 Garden Monitor";

void read_lux_task(void *sensor_param) {
  esp_err_t err;
  float lux;

  for (;;) {
    if ((err = read_lux(&lux)) == ESP_OK)
      ESP_LOGI(TAG, "Lux reading: %f", lux);
    else
      ESP_LOGW(TAG, "Error reading lux: %s", esp_err_to_name(err));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  vTaskDelete(NULL);
}

void read_t_rh_task(void *sensor_param) {
  esp_err_t err;
  float temp, humd;

  for (;;) {
    if ((err = read_temp(&temp)) == ESP_OK)
      ESP_LOGI(TAG, "Temperature reading (C): %f", temp);
    else
      ESP_LOGW(TAG, "Error reading temperature: %s", esp_err_to_name(err));

    if ((err = read_rel_humd(&humd)) == ESP_OK)
      ESP_LOGI(TAG, "Relative humidity reading: %f", humd);
    else
      ESP_LOGW(TAG, "Error reading relative humidity: %s",
               esp_err_to_name(err));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  vTaskDelete(NULL);
}

void read_s_m_task(void *sensor_param) {
  esp_err_t err;
  uint16_t moist;

  for (;;) {
    if ((err = read_soil_moisture(&moist)) == ESP_OK)
      ESP_LOGI(TAG, "Soil moisture reading: %u", moist);
    else
      ESP_LOGW(TAG, "Error reading soil moisture: %s", esp_err_to_name(err));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  vTaskDelete(NULL);
}

void delayed_restart(void) {
  vTaskDelay(pdMS_TO_TICKS(5000));
  esp_restart();
}

void app_main(void) {
  esp_err_t err;
  esp_chip_info_t chip_info;

  printf("Hello world!\n");

  /* Print chip information */
  esp_chip_info(&chip_info);
  printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores,
         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  printf("silicon revision %d, ", chip_info.revision);

  printf("%uMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                       : "external");

  smartconfig__init();

  if ((err = i2c__master_init(I2C_NUM_0, I2C_0_SDA_PIN, I2C_0_SCL_PIN)) !=
      ESP_OK) {
    ESP_LOGE(TAG, "Error initializing I2C bus: %s", esp_err_to_name(err));
    delayed_restart();
  }

  if ((err = init_apds_3901(I2C_NUM_0, APDS_3901_I2C_ADDR)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing lux sensor: %s", esp_err_to_name(err));
    delayed_restart();
  }

  if ((err = init_sht_20(I2C_NUM_0)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing Temp/Humd sensor: %s",
             esp_err_to_name(err));
    delayed_restart();
  }

  if ((err = init_soil_sensor(I2C_NUM_0, SEESAW_I2C_ADDR)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing soil sensor: %s", esp_err_to_name(err));
    delayed_restart();
  }

  // read sensors continuously
  xTaskCreate(&read_lux_task, "read_lux_task", 2048, NULL, 6, NULL);
  xTaskCreate(&read_t_rh_task, "read_t_rh_task", 2048, NULL, 6, NULL);
  xTaskCreate(&read_s_m_task, "read_s_m_task", 2048, NULL, 6, NULL);
}
