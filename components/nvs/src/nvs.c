#include "nvs_flash.h"

#include "../include/nvs.h"

static bool NVS_INIT = false;

void init_nvs(void) {
  esp_err_t err;

  if (NVS_INIT)
    return;

  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  NVS_INIT = true;
}
