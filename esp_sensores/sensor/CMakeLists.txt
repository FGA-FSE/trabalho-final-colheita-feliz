idf_component_register(
    SRCS "dht11.c"
    INCLUDE_DIRS "."
    REQUIRES  driver esp_timer oled_display soil_moisture)
