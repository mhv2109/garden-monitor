#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"

#include "smartconfig.h"
#include "nvs.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;
static SemaphoreHandle_t wifi_mutex;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

/* Logging tag */
static const char *TAG = "smartconfig_wifi";

static void smartconfig_task(void *parm);
static void reconnect_task(void *parm);

static esp_err_t wifi_connect_persisted_config(void);
static esp_err_t wifi_connect_with_config(wifi_config_t *config);

static void
sc_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if (event_base == SC_EVENT) {
    switch (event_id)
    {
    case SC_EVENT_SCAN_DONE:
      ESP_LOGI(TAG, "Scan done");
      break;
    case SC_EVENT_FOUND_CHANNEL:
      ESP_LOGI(TAG, "Found channel");
      break;
    case SC_EVENT_GOT_SSID_PSWD:
      ESP_LOGI(TAG, "Got SSID and password");

      smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
      wifi_config_t wifi_config;
      uint8_t ssid[33] = { 0 };
      uint8_t password[65] = { 0 };

      bzero(&wifi_config, sizeof(wifi_config_t));
      memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
      memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
      wifi_config.sta.bssid_set = evt->bssid_set;
      if (wifi_config.sta.bssid_set == true) {
          memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
      }

      memcpy(ssid, evt->ssid, sizeof(evt->ssid));
      memcpy(password, evt->password, sizeof(evt->password));
      ESP_LOGI(TAG, "SSID:%s", ssid);
      ESP_LOGI(TAG, "PASSWORD:%s", password);
      
      // persist wifi settings
      nvs__set_wifi_config(&wifi_config);

      ESP_ERROR_CHECK(wifi_connect_with_config(&wifi_config));

      break;
    case SC_EVENT_SEND_ACK_DONE:
      xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
      break;
    default:
      break;
    }
  }
}

static void
wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
      xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
	  xTaskCreate(reconnect_task, "reconnect_task", 4096, NULL, 3, NULL);
      break;
    case WIFI_EVENT_STA_DISCONNECTED:
	  ESP_LOGI(TAG, "WiFi disconnected");
      esp_wifi_connect();
      xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
      break;
    default:
      break;
    }
  }
}

static void
ip_event_hanlder(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
  }
}

static void
smartconfig_task(void *parm)
{
    EventBits_t uxBits;
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    
	// let smartconfig run forever -- to allow changing wifi on-line
    for (;;) {
		ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
		ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));

        uxBits = xEventGroupWaitBits(s_wifi_event_group, ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY); 
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
			xEventGroupClearBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
        }

		vTaskDelay(pdMS_TO_TICKS(1000)); // sleep for 1s
    }
}

static void
reconnect_task(void *parm)
{		
	esp_err_t err;
	EventBits_t uxBits;

	for (;;) {
		uxBits = xEventGroupGetBits(s_wifi_event_group);
		if (uxBits & CONNECTED_BIT) {
			break;
		}

		if ((err = wifi_connect_persisted_config()) == ESP_OK) {
			xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
			break;
		} else {
			ESP_LOGW(TAG, "Error (%s) reconnecting to WiFi", esp_err_to_name(err));
		}

		vTaskDelay(pdMS_TO_TICKS(10000)); // retry every 10s
	}

	vTaskDelete(NULL);
}

static esp_err_t
wifi_connect_persisted_config(void)
{
	wifi_config_t config = nvs__get_wifi_config();
	esp_err_t err;
	
	if ((err = wifi_connect_with_config(&config)) == ESP_OK) {
		ESP_LOGI(TAG, "WiFi connected using persisted credentials");
	} else {
		ESP_LOGW(TAG, "Unable to establish WiFi connection with persisted credentials");
	}
	return err;
}

static esp_err_t
wifi_connect_with_config(wifi_config_t *config)
{
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

static void
initialise_wifi(void)
{
  wifi_mutex = xSemaphoreCreateMutex();

  ESP_ERROR_CHECK(esp_netif_init());
  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

  ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL) );
  ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_hanlder, NULL) );
  ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL) );

  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_start() );
}

void 
smartconfig__setup_wifi(void)
{
  nvs__flash_init();
  initialise_wifi();
}