#ifndef NVS_H
#define NVS_H

#include "esp_wifi.h"

void init_nvs(void);

bool get_persisted_wifi_config(wifi_config_t *);
bool set_persisted_wifi_config(wifi_config_t *);

#endif