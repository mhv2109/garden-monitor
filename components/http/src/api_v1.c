#include "../private/api_v1.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <string.h>

#define API_BASE "/api/v1/"
#define API_PATH(resource) strcat(API_BASE, resource)

static esp_err_t get_temp_handler(httpd_req_t *req) {
  esp_err_t err = ESP_OK;
  return err;
}

static esp_err_t get_humd_handler(httpd_req_t *req) {
  esp_err_t err = ESP_OK;
  return err;
}

static esp_err_t get_moist_handler(httpd_req_t *req) {
  esp_err_t err = ESP_OK;
  return err;
}

static esp_err_t get_lux_handler(httpd_req_t *req) {
  esp_err_t err = ESP_OK;
  return err;
}

static const httpd_uri_t get_temp = {.uri = API_PATH("temp"),
                                     .method = HTTP_GET,
                                     .handler = get_temp_handler,
                                     .user_ctx = NULL};

static const httpd_uri_t get_humd = {.uri = API_PATH("humd"),
                                     .method = HTTP_GET,
                                     .handler = get_humd_handler,
                                     .user_ctx = NULL};

static const httpd_uri_t get_moist = {.uri = API_PATH("moist"),
                                      .method = HTTP_GET,
                                      .handler = get_moist_handler,
                                      .user_ctx = NULL};

static const httpd_uri_t get_lux = {.uri = API_PATH("lux"),
                                    .method = HTTP_GET,
                                    .handler = get_lux_handler,
                                    .user_ctx = NULL};

void api_v1__register_handlers(httpd_handle_t handle) {
  httpd_register_uri_handler(handle, &get_temp);
  httpd_register_uri_handler(handle, &get_humd);
  httpd_register_uri_handler(handle, &get_moist);
  httpd_register_uri_handler(handle, &get_lux);
}
