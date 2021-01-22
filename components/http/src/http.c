#include "../include/http.h"
#include "../private/api_v1.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "HTTP Server";

static httpd_handle_t handle = NULL;

esp_err_t start_http_server(void) {
  esp_err_t err = ESP_OK;

  if (handle != NULL)
    return err;

  /* Generate default configuration */
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  /* Local handler to esp_http_server */
  httpd_handle_t handle_ = NULL;

  /* Start the httpd server */
  if ((err = httpd_start(&handle_, &config)) == ESP_OK) {
    /* Register handlers */
    api_v1__register_handlers(handle_);
    /* Set global server var */
    handle = handle_;
    ESP_LOGI(TAG, "Server started");
  } else {
    ESP_LOGE(TAG, "Error starting server: %s", esp_err_to_name(err));
  }

  return err;
}

esp_err_t stop_http_server(void) {
  esp_err_t err = ESP_OK;
  if (handle != NULL) {
    if ((err = httpd_stop(handle)) == ESP_OK)
      ESP_LOGI(TAG, "Server stopped");
    else
      ESP_LOGE(TAG, "Error stopping server: %s", esp_err_to_name(err));
  }
  return err;
}
