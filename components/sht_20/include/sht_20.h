#ifndef SHT_20_H
#define SHT_20_H

#include "esp_err.h"
#include "hal/i2c_types.h"

/// SHT 20's I2C address is non-configurable
#define SHT_20_I2C_ADDR 0x40

/// Registers
#define SHT_20_TEMP_MEASURE_NOHOLD 0xF3
#define SHT_20_HUMD_MEASURE_NOHOLD 0xF5
#define SHT_20_WRITE_USER_REG 0xE6
#define SHT_20_READ_USER_REG 0xE7

/// Configuration constants
#define SHT_20_USER_REGISTER_RESOLUTION_RH12_TEMP14 0x3f

typedef struct sht_20 sht_20;

sht_20* sht_20__new(void);
esp_err_t sht_20__init(i2c_port_t bus, sht_20 *sensor);
esp_err_t sht_20__read_rh(sht_20 *sensor, float *humd);
esp_err_t sht_20__read_t(sht_20 *sensor, float *temp);

#endif
