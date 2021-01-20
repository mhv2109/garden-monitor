#ifndef APDS_3901_H
#define APDS_3901_H

#include "esp_err.h"
#include "hal/i2c_types.h"

/// Register Addresses
#define APDS_3901_CONTROL_REG 0x80
#define APDS_3901_TIMING_REG 0x81
#define APDS_3901_DATA0LOW_REG 0x8c
#define APDS_3901_DATA1LOW_REG 0x8e

/// Config constants
#define APDS_3901_POW_ON 0x3
#define APDS_3901_INT_TIME_402_MS 0x02

esp_err_t init_apds_3901(i2c_port_t bus, uint8_t addr);
esp_err_t read_lux(float *lux);

#endif
