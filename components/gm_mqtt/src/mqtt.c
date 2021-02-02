#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "mqtt_client.h"

#include "../include/mqtt.h"
#include "nvs.h"
#include "sht_20.h"
#include "apds_3901.h"
#include "seesaw_soil.h"

// Config constants
#define ISO_8601_LEN 32
#define BUF_LEN 128

#define TEMPERATURE "temperature"
#define HUMIDITY "humidity"
#define TIME "timestamp"
#define LUX "lux"
#define SOIL_MOISTURE "soil_moisture"

#define BRKR_URI CONFIG_MQTT_BROKER_URI
#define TEMP_TOPIC CONFIG_MQTT_TEMPERATURE_TOPIC
#define HUMD_TOPIC CONFIG_MQTT_HUMIDITY_TOPIC
#define LUX_TOPIC CONFIG_MQTT_LUX_TOPIC
#define SOIL_MOISTURE_TOPIC CONFIG_MQTT_SOIL_MOISTURE_TOPIC

#define ONE_MIN pdMS_TO_TICKS(60000)

static const char* TAG = "mqtt_component";

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
  switch (event->event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "Connected to MQTT broker");
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "Disconnected from MQTT broker");
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGD(TAG, "Published event, message id: %d", event->msg_id);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGD(TAG, "MQTT event, id: %d", event->event_id);
    break;
  }
  return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  mqtt_event_handler_cb(event_data);
}

static esp_mqtt_client_handle_t CLIENT = NULL;

static esp_mqtt_client_handle_t init_mqtt(void) {
  esp_err_t err;
  esp_mqtt_client_handle_t client;
  esp_mqtt_client_config_t mqtt_cfg = {
      .uri = BRKR_URI,
  };

  if (CLIENT != NULL)
    return CLIENT;

  // initialize dependencies
  esp_event_loop_create_default(); // may or may not already be initialized
  init_nvs();

  // initialize mqtt client
  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
  if ((err = esp_mqtt_client_start(client)) != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing client: %s", esp_err_to_name(err));
    return NULL;
  }

  CLIENT = client;
  return CLIENT;
}

static void get_utc_iso_8601(char *t) {
  time_t now = time(&now);
  struct tm *gmt = gmtime(&now);
  strftime(t, ISO_8601_LEN, "%FT%TZ", gmt);
}

static void json_float(char *buf, const char* key, float val) {
  char ts[ISO_8601_LEN] = {0};
  get_utc_iso_8601(ts);
  sprintf(buf, "{\"%s\":%f,\"%s\":\"%s\"}", key, val, TIME, ts);
}

static void json_uint16(char *buf, const char* key, uint16_t val) {
  char ts[ISO_8601_LEN] = {0};
  get_utc_iso_8601(ts);
  sprintf(buf, "{\"%s\":%u,\"%s\":\"%s\"}", key, val, TIME, ts);
}

static void read_temp_task(void *client) {
  esp_err_t err;
  float temp;
  char payload[BUF_LEN] = {0};

  client = (esp_mqtt_client_handle_t*)client;

  for (;;) {
    if ((err = read_temp(&temp)) == ESP_OK) {
      json_float(payload, TEMPERATURE, temp);
      if (esp_mqtt_client_publish(client, TEMP_TOPIC, payload, 0, 1, 1) < 0)
        ESP_LOGW(TAG, "Error publishing temperature message");
    } else {
      ESP_LOGE(TAG, "Error reading temperature: %s", esp_err_to_name(err));
    }
    vTaskDelay(ONE_MIN);
  }

  vTaskDelete(NULL);
}

static bool TEMP_INIT = false;

void mqtt_publish_temp(void) {
  esp_mqtt_client_handle_t client;

  if (TEMP_INIT)
    return;

  client = init_mqtt();
  xTaskCreate(&read_temp_task, "read_temp_task", 2048, client, 6, NULL);

  TEMP_INIT = true;
}

static void read_humd_task(void *client) {
  esp_err_t err;
  float humd;
  char payload[BUF_LEN] = {0};

  client = (esp_mqtt_client_handle_t*)client;

  for (;;) {
    if ((err = read_rel_humd(&humd)) == ESP_OK) {
      json_float(payload, HUMIDITY, humd);
      if (esp_mqtt_client_publish(client, HUMD_TOPIC, payload, 0, 1, 1) < 0)
        ESP_LOGW(TAG, "Error publishing humidity message");
    } else {
      ESP_LOGE(TAG, "Error reading humidity: %s", esp_err_to_name(err));
    }
    vTaskDelay(ONE_MIN);
  }

  vTaskDelete(NULL);
}

static bool HUMD_INIT = false;

void mqtt_publish_humd(void) {
  esp_mqtt_client_handle_t client;

  if (HUMD_INIT)
    return;

  client = init_mqtt();
  xTaskCreate(&read_humd_task, "read_humd_task", 2048, client, 6, NULL);

  HUMD_INIT = true;
}

static void read_lux_task(void *client) {
  esp_err_t err;
  float lux;
  char payload[BUF_LEN] = {0};

  client = (esp_mqtt_client_handle_t*)client;

  for (;;) {
    if ((err = read_lux(&lux)) == ESP_OK) {
      json_float(payload, LUX, lux);
      if (esp_mqtt_client_publish(client, LUX_TOPIC, payload, 0, 1, 1) < 0)
        ESP_LOGW(TAG, "Error publishing lux message");
    } else {
      ESP_LOGE(TAG, "Error reading lux: %s", esp_err_to_name(err));
    }
    vTaskDelay(ONE_MIN);
  }

  vTaskDelete(NULL);
}

static bool LUX_INIT = false;

void mqtt_publish_lux(void) {
  esp_mqtt_client_handle_t client;

  if (LUX_INIT)
    return;

  client = init_mqtt();
  xTaskCreate(&read_lux_task, "read_lux_task", 2048, client, 6, NULL);

  LUX_INIT = true;
}

static void read_soil_moisture_task(void *client) {
  esp_err_t err;
  uint16_t moist;
  char payload[BUF_LEN] = {0};

  client = (esp_mqtt_client_handle_t*)client;

  for (;;) {
    if ((err = read_soil_moisture(&moist)) == ESP_OK) {
      json_uint16(payload, SOIL_MOISTURE, moist);
      if (esp_mqtt_client_publish(client, SOIL_MOISTURE_TOPIC, payload, 0, 1, 1) < 0)
        ESP_LOGW(TAG, "Error publishing soil moisture message");
    } else {
      ESP_LOGE(TAG, "Error reading soil moisture: %s", esp_err_to_name(err));
    }
    vTaskDelay(ONE_MIN);
  }

  vTaskDelete(NULL);
}

static bool MOIST_INIT = false;

void mqtt_publish_moist(void) {
  esp_mqtt_client_handle_t client;

  if (MOIST_INIT)
    return;

  client = init_mqtt();
  xTaskCreate(&read_soil_moisture_task, "read_soil_moisture_task", 2048, client, 6, NULL);

  MOIST_INIT = true;
}

void mqtt_publish_all(void) {
  mqtt_publish_temp();
  mqtt_publish_humd();
  mqtt_publish_lux();
  mqtt_publish_moist();
}
