#ifndef WIFI_H
#define WIFI_H

#include "esp_system.h"
#include "esp_wifi.h"

void init_wifi(void);
esp_err_t wifi_connect(wifi_config_t *);

#endif