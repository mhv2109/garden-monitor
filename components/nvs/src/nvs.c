#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "../include/nvs.h"
#include "../private/nvs_private.h"

#define READ_ERROR_CHECK(err)                                                  \
  switch (err) {                                                               \
  case ESP_OK:                                                                 \
    break;                                                                     \
  case ESP_ERR_NVS_NOT_FOUND:                                                  \
    return false;                                                              \
  default:                                                                     \
    ESP_LOGE(TAG, "Error (%s) reading from NVS", esp_err_to_name(err));        \
    return false;                                                              \
  }

#define WRITE_ERROR_CHECK(err)                                                 \
  if (err != ESP_OK) {                                                         \
    ESP_LOGE(TAG, "Error (%s) writing to NVS", esp_err_to_name(err));          \
    return false;                                                              \
  }

static const char *TAG = "nvs_component"; // logging tag
static bool NVS_INITIALIZED =
    false; // has the NVS flash system been initialized?

bool read_string(const char *ns, const char *key, char *result) {
  nvs_handle_t handle;
  size_t len;

  if (!NVS_INITIALIZED)
    nvs__flash_init();

  READ_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
  READ_ERROR_CHECK(nvs_get_str(handle, key, NULL, &len));
  READ_ERROR_CHECK(nvs_get_str(handle, key, result, &len));

  return true;
}

bool set_string(const char *ns, const char *key, char *value) {
  nvs_handle_t handle;

  if (!NVS_INITIALIZED)
    nvs__flash_init();

  WRITE_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
  WRITE_ERROR_CHECK(nvs_set_str(handle, key, value));

  return true;
}

bool read_int8(const char *ns, const char *key, int8_t *result) {
  nvs_handle_t handle;

  if (!NVS_INITIALIZED)
    nvs__flash_init();

  READ_ERROR_CHECK(nvs_open(ns, NVS_READONLY, &handle));
  READ_ERROR_CHECK(nvs_get_i8(handle, key, result));

  return true;
}

bool set_int8(const char *ns, const char *key, int8_t value) {
  nvs_handle_t handle;

  if (!NVS_INITIALIZED)
    nvs__flash_init();

  WRITE_ERROR_CHECK(nvs_open(ns, NVS_READWRITE, &handle));
  WRITE_ERROR_CHECK(nvs_set_i8(handle, key, value));

  return true;
}

/**
 * Initialize NVS Flash system.  NVS_INITIALIZED ensures that this only runs
 * once.
 **/
void nvs__flash_init(void) {
  if (NVS_INITIALIZED)
    return;

  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // don't run more than once
  NVS_INITIALIZED = true;
}