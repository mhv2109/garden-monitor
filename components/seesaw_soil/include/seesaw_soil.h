#ifndef SEESAW_SOIL_H
#define SEESAW_SOIL_H

#include "esp_err.h"
#include "hal/i2c_types.h"

/// Register addresses
#define SEESAW_TOUCH_BASE 0x0f
#define SEESAW_TOUCH_CHANNEL_OFFSET 0x10
#define SEESAW_TOUCH_PIN 0x00

typedef struct seesaw_soil {
  i2c_port_t bus;
  uint8_t addr;
} seesaw_soil;

esp_err_t seesaw__init(i2c_port_t bus, uint8_t addr, seesaw_soil *sensor);
esp_err_t seesaw__read_m(seesaw_soil *sensor, uint16_t *moist);

#endif
