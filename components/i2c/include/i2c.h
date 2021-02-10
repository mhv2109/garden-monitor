#ifndef I2C_H
#define I2C_H

#include "esp_err.h"
#include "hal/i2c_types.h"
#include "driver/i2c.h"

#if CONFIG_SDA_NUM_2
#define I2C_SDA_PIN GPIO_NUM_2
#elif CONFIG_SDA_NUM_15
#define I2C_SDA_PIN GPIO_NUM_15
#elif CONFIG_SDA_NUM_21
#define I2C_SDA_PIN GPIO_NUM_21
#elif CONFIG_SDA_NUM_22
#define I2C_SDA_PIN GPIO_NUM_22
#elif CONFIG_SDA_NUM_23
#define I2C_SDA_PIN GPIO_NUM_23
#else
#define I2C_SDA_PIN GPIO_NUM_23
#endif

#if CONFIG_SCL_NUM_2
#define I2C_SCL_PIN GPIO_NUM_2
#elif CONFIG_SCL_NUM_15
#define I2C_SCL_PIN GPIO_NUM_15
#elif CONFIG_SCL_NUM_21
#define I2C_SCL_PIN GPIO_NUM_21
#elif CONFIG_SCL_NUM_22
#define I2C_SCL_PIN GPIO_NUM_22
#elif CONFIG_SCL_NUM_23
#define I2C_SCL_PIN GPIO_NUM_23
#else
#define I2C_SCL_PIN GPIO_NUM_22
#endif

#define I2C_BUS I2C_NUM_0

esp_err_t init_i2c_master();

#endif
