idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt_config nvs_flash esp_wifi wifi_config
                       REQUIRES mqtt button  esp_wifi)
