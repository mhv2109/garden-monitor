idf_component_register(
  SRCS "smartconfig.c"
  INCLUDE_DIRS "."
  REQUIRES esp_wifi wpa_supplicant nvs) 