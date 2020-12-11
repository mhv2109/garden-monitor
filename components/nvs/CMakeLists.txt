idf_component_register(
  SRCS "src/nvs.c" "src/wifi.c" 
  INCLUDE_DIRS "include"
  REQUIRES nvs_flash esp_wifi)