#ifndef NVS_H
#define NVS_H

#include "esp_wifi.h"

#include "webhook.h"

void nvs__flash_init(void);

bool nvs__get_wifi_config(wifi_config_t *);
bool nvs__set_wifi_config(wifi_config_t *);

bool nvs__get_webhooks(webhooks *ws);
bool nvs__save_webhook(webhook *w);
bool nvs__del_webhook(webhook *w);

#endif
