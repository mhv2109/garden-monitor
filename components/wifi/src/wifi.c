#include "esp_event.h"
#include "esp_interface.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_wpa2.h"
#include "esp_sntp.h"

#include "../include/wifi.h"
#include "nvs.h"

// Config constants
#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASS
static const char *TAG = "wifi_component";

// Global vars
static bool WIFI_INIT = false;

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
    case IP_EVENT_STA_LOST_IP:
      ESP_LOGW(TAG, "Network disconnected, IP address lost");
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
      esp_wifi_connect();
      break;
    case WIFI_EVENT_WIFI_READY:
      ESP_LOGI(TAG, "WiFi ready");
      break;
    case WIFI_EVENT_SCAN_DONE:
      ESP_LOGI(TAG, "WiFi scan done");
      break;
    case WIFI_EVENT_STA_CONNECTED:
      ESP_LOGI(TAG, "WiFi station connected to AP");
      break;
    case WIFI_EVENT_STA_DISCONNECTED:
      ESP_LOGI(TAG, "WiFi disconnected from AP");
      esp_wifi_connect();
      break;
    default:
      break;
    }
  }
}

/**
 * @brief Initialize ESP32 WiFi system.
 * @note Panics on failure.
 */
void init_wifi(void) {
  if (WIFI_INIT)
    return;

  init_nvs();

  ESP_ERROR_CHECK(esp_netif_init());
  esp_event_loop_create_default(); // may or may not already be initialized
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &ip_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP,
                                             &ip_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));

  wifi_config_t wifi_config = {
      .sta = {.ssid = WIFI_SSID,
              .password = WIFI_PASS,
              .scan_method = WIFI_ALL_CHANNEL_SCAN,
              .pmf_cfg = {.capable = true, .required = false},
              .threshold.authmode = WIFI_AUTH_WPA2_PSK},
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  WIFI_INIT = true;
}
