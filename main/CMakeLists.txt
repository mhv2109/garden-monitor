idf_component_register(
    SRCS "src/main.c"
    INCLUDE_DIRS ""
	REQUIRES nvs wifi smartconfig i2c apds_3901 sht_20 seesaw_soil http)
