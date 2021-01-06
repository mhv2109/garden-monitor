#ifndef APDS_3901_H
#define APDS_3901_H

#include "esp_err.h"
#include "hal/i2c_types.h"

typedef struct apds_3901 {
  i2c_port_t bus;
  uint8_t addr;
  bool p_on;
} apds_3901;

esp_err_t apds_3901__init(i2c_port_t bus, uint8_t addr, apds_3901 *sensor);
esp_err_t apds_3901__read_lux(apds_3901 *sensor, float *lux);

#endif
