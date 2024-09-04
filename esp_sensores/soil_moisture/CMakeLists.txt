idf_component_register(SRCS "soil_moisture.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_adc oled_display)
