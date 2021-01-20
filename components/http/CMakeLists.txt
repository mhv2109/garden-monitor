idf_component_register(
  SRCS "src/http.c"
  INCLUDE_DIRS "include"
  REQUIRES esp_http_server apds_3901 seesaw_soil sht_20)
