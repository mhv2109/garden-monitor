#include "../private/api_v1.h"
#include "apds_3901.h"
#include "seesaw_soil.h"
#include "sht_20.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BUF_LEN 128
#define ISO_8601_LEN 32

#define API_BASE "/api/v1/"

#define TEMP "temperature"
#define HUMD "humidity"
#define MOIST "soil-moisture"
#define LUX "lux"
#define ERROR "error"
#define TIME "timestamp"

const char *TAG = "REST API v1";

void get_utc_iso_8601(char *t) {
  time_t now = time(&now);
  struct tm *gmt = gmtime(&now);
  strftime(t, ISO_8601_LEN, "%FT%TZ", gmt);
}

static esp_err_t get_temp_handler(httpd_req_t *req) {
  esp_err_t err;
  float temp;
  char ts[ISO_8601_LEN] = {0};
  char buf[BUF_LEN] = {0};

  get_utc_iso_8601(ts);

  if ((err = read_temp(&temp)) == ESP_OK) {
    sprintf(buf, "{\"%s\":%f,\"%s\":\"%s\"}", TEMP, temp, TIME, ts);
  } else {
    ESP_LOGE(TAG, "Error returning current temperature: %s", esp_err_to_name(err));
    sprintf(buf, "{\"%s\":\"%s\",\"%s\":\"%s\"}", ERROR, esp_err_to_name(err), TIME, ts);
  }

  httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  return httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t get_humd_handler(httpd_req_t *req) {
  esp_err_t err;
  float humd;
  char ts[ISO_8601_LEN] = {0};
  char buf[BUF_LEN] = {0};

  get_utc_iso_8601(ts);

  if ((err = read_rel_humd(&humd)) == ESP_OK) {
    sprintf(buf, "{\"%s\":%f,\"%s\":\"%s\"}", HUMD, humd, TIME, ts);
  } else {
    ESP_LOGE(TAG, "Error returning current relative humidity: %s", esp_err_to_name(err));
    sprintf(buf, "{\"%s\":\"%s\",\"%s\":\"%s\"}", ERROR, esp_err_to_name(err), TIME, ts);
  }

  httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  return httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t get_moist_handler(httpd_req_t *req) {
  esp_err_t err;
  uint16_t moist;
  char ts[ISO_8601_LEN] = {0};
  char buf[BUF_LEN] = {0};

  get_utc_iso_8601(ts);

  if ((err = read_soil_moisture(&moist)) == ESP_OK) {
    sprintf(buf, "{\"%s\":%u,\"%s\":\"%s\"}", MOIST, moist, TIME, ts);
  } else {
    ESP_LOGE(TAG, "Error returning soil moisture: %s", esp_err_to_name(err));
    sprintf(buf, "{\"%s\":\"%s\",\"%s\":\"%s\"}", ERROR, esp_err_to_name(err), TIME, ts);
  }

  httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  return httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t get_lux_handler(httpd_req_t *req) {
  esp_err_t err;
  float lux;
  char ts[ISO_8601_LEN] = {0};
  char buf[BUF_LEN] = {0};

  get_utc_iso_8601(ts);

  if ((err = read_lux(&lux)) == ESP_OK) {
    sprintf(buf, "{\"%s\":%f,\"%s\":\"%s\"}", LUX, lux, TIME, ts);
  } else {
    ESP_LOGE(TAG, "Error returning lux: %s", esp_err_to_name(err));
    sprintf(buf, "{\"%s\":\"%s\",\"%s\":\"%s\"}", ERROR, esp_err_to_name(err), TIME, ts);
  }

  httpd_resp_set_type(req, HTTPD_TYPE_JSON);
  return httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
}

static const httpd_uri_t get_temp = {.uri = API_BASE TEMP,
                                     .method = HTTP_GET,
                                     .handler = get_temp_handler,
                                     .user_ctx = NULL};

static const httpd_uri_t get_humd = {.uri = API_BASE HUMD,
                                     .method = HTTP_GET,
                                     .handler = get_humd_handler,
                                     .user_ctx = NULL};

static const httpd_uri_t get_moist = {.uri = API_BASE MOIST,
                                      .method = HTTP_GET,
                                      .handler = get_moist_handler,
                                      .user_ctx = NULL};

static const httpd_uri_t get_lux = {.uri = API_BASE LUX,
                                    .method = HTTP_GET,
                                    .handler = get_lux_handler,
                                    .user_ctx = NULL};

void api_v1__register_handlers(httpd_handle_t handle) {
  httpd_register_uri_handler(handle, &get_temp);
  httpd_register_uri_handler(handle, &get_humd);
  httpd_register_uri_handler(handle, &get_moist);
  httpd_register_uri_handler(handle, &get_lux);
}
