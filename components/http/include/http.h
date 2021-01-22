#ifndef HTTP_H
#define HTTP_H

#include "esp_err.h"

esp_err_t start_http_server(void);
esp_err_t stop_http_server(void);

#endif
