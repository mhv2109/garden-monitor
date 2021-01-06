#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <string.h>

#include "nvs.h"
#include "wifi.h"

#include "../include/smartconfig.h"

/* FreeRTOS event group to signal when we are connected & ready to make a
 * request */
static EventGroupHandle_t s_wifi_event_group;

/* The only event we care about is "have we completed SmartConfig setup?" */
static const int ESPTOUCH_DONE_BIT = BIT0;

/* Logging tag */
static const char *TAG = "smartconfig_component";

/* Only allow SmartConfig to be initialized once */
static bool SC_INIT = false;

static void smartconfig_task(void *parm) {
  EventBits_t uxBits;
  smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();

  ESP_LOGI(TAG, "Starting SmartConfig");

  // let smartconfig run forever -- to allow changing wifi on-line
  for (;;) {
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));

    uxBits = xEventGroupWaitBits(s_wifi_event_group, ESPTOUCH_DONE_BIT, true,
                                 false, portMAX_DELAY);
    if (uxBits & ESPTOUCH_DONE_BIT) {
      ESP_LOGI(TAG, "SmartConfig complete");
      esp_smartconfig_stop();
      xEventGroupClearBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }

    vTaskDelay(pdMS_TO_TICKS(1000)); // sleep for 1s
  }
}

static void sc_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
  if (event_base == SC_EVENT) {
    switch (event_id) {
    case SC_EVENT_SCAN_DONE:
      ESP_LOGI(TAG, "Scan done");
      break;
    case SC_EVENT_FOUND_CHANNEL:
      ESP_LOGI(TAG, "Found channel");
      break;
    case SC_EVENT_GOT_SSID_PSWD:
      ESP_LOGI(TAG, "Got SSID and password");

      smartconfig_event_got_ssid_pswd_t *evt =
          (smartconfig_event_got_ssid_pswd_t *)event_data;
      wifi_config_t wifi_config;
      uint8_t ssid[33] = {0};
      uint8_t password[65] = {0};

      bzero(&wifi_config, sizeof(wifi_config_t));
      memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
      memcpy(wifi_config.sta.password, evt->password,
             sizeof(wifi_config.sta.password));
      wifi_config.sta.bssid_set = evt->bssid_set;
      if (wifi_config.sta.bssid_set == true) {
        memcpy(wifi_config.sta.bssid, evt->bssid,
               sizeof(wifi_config.sta.bssid));
      }

      memcpy(ssid, evt->ssid, sizeof(evt->ssid));
      memcpy(password, evt->password, sizeof(evt->password));
      ESP_LOGD(TAG, "SSID:%s", ssid);
      ESP_LOGD(TAG, "PASSWORD:%s", password);

      // persist wifi settings
      nvs__set_wifi_config(&wifi_config);

      ESP_ERROR_CHECK(wifi__connect_with_config(&wifi_config));

      break;
    case SC_EVENT_SEND_ACK_DONE:
      xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
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
      xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
      break;
    default:
      break;
    }
  }
}

void smartconfig__init(void) {
  if (SC_INIT)
    return;

  esp_event_loop_create_default(); // may or may not already be started, ignore
                                   // error
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID,
                                             &sc_event_handler, NULL));

  wifi__init();

  SC_INIT = true;
}