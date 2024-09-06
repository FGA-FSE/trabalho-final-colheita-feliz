idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt_config sensor wifi_config json oled_display soil_moisture water_sensor relay    
                       REQUIRES esp_event nvs_flash esp_wifi mqtt oled_display soil_moisture  esp_adc )
