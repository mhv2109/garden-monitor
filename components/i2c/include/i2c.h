#ifndef I2C_H
#define I2C_H

#include "esp_err.h"
#include "hal/i2c_types.h"

esp_err_t init_i2c_master(i2c_port_t bus, int sda_io_num, int scl_io_num);

#endif
