#include "driver/i2c.h"
#include "../include/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/i2c_types.h"

static const char *TAG = "i2c_component";

static bool bus_0_init = false;
static bool bus_1_init = false;

/**
 * Initialize one of two available I2C busses using the provided pins for
 * SDA and SCL. Function will not allow re-initializing either bus.
 * @param bus one of two I2C buses on ESP32 chip
 * @param sda_io_num GPIO pin for I2C SDA line
 * @param scl_io_num GPIO pin for I2C SCL line
 * @return error
 */
esp_err_t i2c__master_init(i2c_port_t bus, int sda_io_num, int scl_io_num) {
  esp_err_t err = ESP_OK;
  i2c_config_t i2c_conf = {.mode = I2C_MODE_MASTER,
                           .sda_io_num = sda_io_num,
                           .scl_io_num = scl_io_num,
                           .sda_pullup_en = GPIO_PULLUP_ENABLE,
                           .scl_pullup_en = GPIO_PULLUP_ENABLE,
                           .master.clk_speed = 400000};

  // don't allow re-initialization of I2C busses
  if ((bus == I2C_NUM_0 && bus_0_init) || (bus == I2C_NUM_1 && bus_1_init))
    return err;

  if ((err = i2c_param_config(bus, &i2c_conf)) != ESP_OK)
    return err;

  if ((err = i2c_driver_install(bus, I2C_MODE_MASTER, 0, 0, 0)) != ESP_OK)
    return err;

  if (bus == I2C_NUM_0)
    bus_0_init = true;
  else if (bus == I2C_NUM_1)
    bus_1_init = true;

  ESP_LOGI(TAG, "I2C Bus %d initialized", bus);
  return err;
}
