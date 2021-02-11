#ifndef BATT_H
#define BATT_H

#include "driver/adc.h"

#if CONFIG_BATT_ADC1_CHANNEL_2
#define BATT_ADC_CHANNEL ADC1_CHANNEL_2
#elif CONFIG_BATT_ADC1_CHANNEL_6
#define BATT_ADC_CHANNEL ADC1_CHANNEL_6
#else
#define BATT_ADC_CHANNEL ADC1_CHANNEL_6
#endif

#if CONFIG_BATT_ADC_DEFAULT_VREF
#define BATT_ADC_DEFAULT_VREF CONFIG_BATT_ADC_DEFAULT_VREF
#else
#define BATT_ADC_DEFAULT_VREF 1100
#endif

#define BATT_ADC_WIDTH_BIT ADC_WIDTH_BIT_12
#define BATT_ADC_ATTEN ADC_ATTEN_DB_11 // expected input is between 1.5-2.1v
#define BATT_ADC_UNIT ADC_UNIT_1
#define BATT_ADC_N_SAMPLES 64 

esp_err_t init_batt_adc(void);
esp_err_t read_batt(uint32_t *);

#endif