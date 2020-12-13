idf_component_register(
  SRCS "src/smartconfig.c"
  INCLUDE_DIRS "include"
  REQUIRES esp_wifi wpa_supplicant nvs wifi) 