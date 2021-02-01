#ifndef SEESAW_SOIL_H
#define SEESAW_SOIL_H

#include "esp_err.h"
#include "hal/i2c_types.h"

/// Register addresses
#define SEESAW_TOUCH_BASE 0x0f
#define SEESAW_TOUCH_CHANNEL_OFFSET 0x10
#define SEESAW_TOUCH_PIN 0x00

esp_err_t init_soil_sensor(i2c_port_t bus, uint8_t addr);
esp_err_t read_soil_moisture(uint16_t *moist);

#endif
