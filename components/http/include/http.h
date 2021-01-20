#ifndef HTTP_H
#define HTTP_H

#include "esp_err.h"

esp_err_t http__start_webserver(void);
esp_err_t http__stop_webserver(void);

#endif
