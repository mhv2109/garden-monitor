#include "../include/batt.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "Battery Monitor";
static esp_adc_cal_characteristics_t *adc_chars = NULL;
static bool batt_adc_init = false;

/**
 * @brief Initialize ADC1 and configured pin for reading battery voltage.
 * @return error
 */
esp_err_t init_batt_adc(void) {
  esp_err_t err = ESP_OK;

  if (batt_adc_init)
    return err;

  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(BATT_ADC_UNIT, BATT_ADC_ATTEN, BATT_ADC_WIDTH_BIT,
                           BATT_ADC_DEFAULT_VREF, adc_chars);

  if ((err = adc1_config_width(BATT_ADC_WIDTH_BIT)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing ADC1 bit width: %s",
             esp_err_to_name(err));
    return err;
  }

  if ((err = adc1_config_channel_atten(BATT_ADC_CHANNEL, BATT_ADC_ATTEN)) !=
      ESP_OK) {

    ESP_LOGE(TAG, "Error initializing ADC1 channel attenuation: %s",
             esp_err_to_name(err));
    return err;
  }

  return err;
}

/**
 * @brief Read battery voltage on configured pin using ADC1.
 * @param voltage return-arg for voltage reading
 * @return error
 */
esp_err_t read_batt(uint32_t *voltage) {
  esp_err_t err = ESP_OK;
  uint32_t sum = 0, reading;

  if (!batt_adc_init)
    if ((err = init_batt_adc()) != ESP_OK)
      return err;

  for (int i = 0; i < BATT_ADC_N_SAMPLES; i++) {
    reading = 0;
    if ((err = esp_adc_cal_get_voltage(BATT_ADC_CHANNEL, adc_chars,
                                       &reading)) != ESP_OK) {
      ESP_LOGE(TAG, "Error reading ADC: %s", esp_err_to_name(err));
      return err;
    }
    sum += reading;
  }

  *voltage = sum / BATT_ADC_N_SAMPLES;
  *voltage *= 2; // using a voltage halving circuit

  return ESP_OK;
}