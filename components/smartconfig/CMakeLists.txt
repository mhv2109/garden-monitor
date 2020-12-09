idf_component_register(
  SRCS "smartconfig.c"
  INCLUDE_DIRS "."
  REQUIRES esp_wifi nvs_flash wpa_supplicant) 