idf_component_register(
    SRCS "dht11.c" "waterSensor.c" "adc_module.c"
    INCLUDE_DIRS "."
    REQUIRES  driver esp_timer esp_adc mqtt mqtt_config)
