#ifndef WIFI_H
#define WIFI_H

#include "esp_system.h"
#include "esp_wifi.h"

void wifi__init(void);
esp_err_t wifi__connect_with_config(wifi_config_t *);
bool wifi__connected(void);

#endif
