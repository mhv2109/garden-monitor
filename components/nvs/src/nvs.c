#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "../private/nvs_private.h"
#include "../include/nvs.h"

static const char *TAG = "nvs_component"; // logging tag
static uint8_t NVS_INITIALIZED = 0; // has the NVS flash system been initialized?

char*
read_string(const char *ns, const char *key)
{
  nvs_handle_t handle;
  esp_err_t err;
  size_t len;

  if (!NVS_INITIALIZED)
    nvs__flash_init();
  
  err = nvs_open(ns, NVS_READONLY, &handle);
  switch (err) {
    case ESP_OK:
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      return NULL;
    default:
      ESP_LOGE(TAG, "Error (%s) reading from NVS", esp_err_to_name(err));
      return NULL;
  }  

  err = nvs_get_str(handle, key, NULL, &len);
  switch (err) {
  case ESP_OK:
    // a-ok
    break;
  case ESP_ERR_NVS_NOT_FOUND:
    // key not found, return NULL for value
    return NULL;
  default:
    // we've got problems
    ESP_LOGE(TAG, "Error (%s) reading from NVS", esp_err_to_name(err));
    return NULL;
  }

  char *str = (char *) malloc(len * sizeof(char));
  err = nvs_get_str(handle, key, str, &len);
  if (err != ESP_OK) {
    // can't imagine when we'd get ESP_OK above and fail here
    ESP_LOGE(TAG, "Error (%s) reading from NVS", esp_err_to_name(err));
    return NULL;
  }

  return str;
}

void
set_string(const char *ns, const char *key, char *value)
{
  nvs_handle_t handle;
  esp_err_t err;

  if (!NVS_INITIALIZED)
    nvs__flash_init();
  
  err = nvs_open(ns, NVS_READWRITE, &handle);
  ESP_ERROR_CHECK(err);

  err = nvs_set_str(handle, key, value);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "Error (%s) writing to NVS", esp_err_to_name(err));
}

int8_t
read_int8(const char *ns, const char *key)
{
  nvs_handle_t handle;
  esp_err_t err;
  int8_t res;

  if (!NVS_INITIALIZED)
    nvs__flash_init();
  
  err = nvs_open(ns, NVS_READONLY, &handle);
  switch (err) {
    case ESP_OK:
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      return -1;
    default:
      ESP_LOGE(TAG, "Error (%s) reading from NVS", esp_err_to_name(err));
      return -1;
  }  

  err = nvs_get_i8(handle, key, &res);
  switch (err)
  {
  case ESP_OK:
    break;
  case ESP_ERR_NVS_NOT_FOUND:
    res = -1; 
    break;
  default:
    ESP_LOGE(TAG, "Error (%s) reading from NVS", esp_err_to_name(err)); 
    res = -1;
    break;
  } 

  return res;
}

void
set_int8(const char *ns, const char *key, int8_t value)
{
  nvs_handle_t handle;
  esp_err_t err;

  if (!NVS_INITIALIZED)
    nvs__flash_init();

  err = nvs_open(ns, NVS_READWRITE, &handle);
  ESP_ERROR_CHECK(err);

  err = nvs_set_i8(handle, key, value);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "Error (%s) writing to NVS", esp_err_to_name(err));
}

/**
 * Initialize NVS Flash system.  NVS_INITIALIZED ensures that this only runs
 * once.
 **/
void
nvs__flash_init(void)
{
  if (!NVS_INITIALIZED) {
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    // don't run more than once
    NVS_INITIALIZED = 1;
    ESP_LOGI(TAG, "NVS initialized");
  } else {
    ESP_LOGD(TAG, "NVS already initialized, skipping");
  }
}