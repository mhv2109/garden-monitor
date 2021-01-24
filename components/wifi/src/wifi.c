#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_sntp.h"

#include "nvs.h"

#include "../include/wifi.h"

/* Logging tag */
static const char *TAG = "wifi_component";

/* Prevent concurrent overwrites of WiFi settings */
static SemaphoreHandle_t wifi_mutex;

/* Only allow wifi to be initialized once, and make sure it is before
interacting with the WiFi/network stack */
static bool WIFI_INIT = false;

/**
 * @brief Connect to WiFi using the provided config.
 * @param config ESP-IDF WiFi config with ssid, password, and bssid set
 * @return error
 */
esp_err_t wifi__connect_with_config(wifi_config_t *config) {
  if (!WIFI_INIT)
    wifi__init();

  esp_err_t err;

  xSemaphoreTake(wifi_mutex, portMAX_DELAY);

  if ((err = esp_wifi_disconnect()) != ESP_OK)
    goto exit;
  if ((err = esp_wifi_set_config(ESP_IF_WIFI_STA, config)) != ESP_OK)
    goto exit;
  err = esp_wifi_connect();

exit:
  xSemaphoreGive(wifi_mutex);
  return err;
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
  if (event_base == IP_EVENT) {
    switch (event_id) {
    case IP_EVENT_STA_GOT_IP:
      ESP_LOGI(TAG, "Network connected, IP address assigned");
      // setup ntp server
      sntp_setoperatingmode(SNTP_OPMODE_POLL);
      sntp_setservername(0, "pool.ntp.org");
      sntp_init();
      break;
    default:
      break;
    }
  }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
      ESP_LOGI(TAG, "WiFi started");
      break;
    case WIFI_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "WiFi disconnected");
      esp_wifi_connect();
      break;
    default:
      break;
    }
  }
}

static esp_err_t connect_persisted_config(void) {
  wifi_config_t config;
  esp_err_t err;

  nvs__get_wifi_config(&config);

  if ((err = wifi__connect_with_config(&config)) == ESP_OK) {
    ESP_LOGI(TAG, "WiFi connected using persisted credentials");
  } else {
    ESP_LOGW(TAG,
             "Unable to establish WiFi connection with persisted credentials");
  }
  return err;
}

/**
 * @brief Initialize ESP32 WiFi system.
 * @note Panics on failure.
 */
void wifi__init(void) {
  if (WIFI_INIT)
    return;

  nvs__flash_init();

  wifi_mutex = xSemaphoreCreateMutex();

  ESP_ERROR_CHECK(esp_netif_init());
  esp_event_loop_create_default(); // may or may not already be started, ignore
                                   // error
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &ip_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  WIFI_INIT = true;

  connect_persisted_config();
}
