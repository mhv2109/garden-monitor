idf_component_register(
  SRCS "src/http.c" "src/webhook.c" "src/api_v1.c" 
  INCLUDE_DIRS "include"
  REQUIRES esp_http_server nvs apds_3901 seesaw_soil sht_20)
