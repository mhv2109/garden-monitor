#include "driver/i2c.h"
#include "../include/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/i2c_types.h"

static const char *TAG = "i2c_component";

static bool i2c_init = false;

/**
 * Initializes I2C busses using pins configured uisng Kconfig.
 * @return error
 */
esp_err_t init_i2c_master() {
  esp_err_t err = ESP_OK;
  i2c_config_t i2c_conf = {.mode = I2C_MODE_MASTER,
                           .sda_io_num = I2C_SDA_PIN,
                           .scl_io_num = I2C_SCL_PIN,
                           .sda_pullup_en = GPIO_PULLUP_ENABLE,
                           .scl_pullup_en = GPIO_PULLUP_ENABLE,
                           .master.clk_speed = 400000};

  // don't allow re-initialization of I2C busses
  if (i2c_init)
    return err;

  if ((err = i2c_param_config(I2C_BUS, &i2c_conf)) != ESP_OK)
    return err;

  if ((err = i2c_driver_install(I2C_BUS, I2C_MODE_MASTER, 0, 0, 0)) != ESP_OK)
    return err;

  i2c_init = true;
  ESP_LOGI(TAG, "I2C Bus %d initialized", I2C_BUS);
  return err;
}
