idf_component_register(SRCS "mqtt.c"
                       INCLUDE_DIRS "."
                       REQUIRES mqtt button esp_wifi nvs_flash wifi_config)
