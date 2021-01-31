idf_component_register(
  SRCS "src/mqtt.c"
  INCLUDE_DIRS "include"
  REQUIRES nvs mqtt apds_3901 seesaw_soil sht_20)
