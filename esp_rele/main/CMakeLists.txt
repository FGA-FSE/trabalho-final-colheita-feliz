idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt_config  wifi_config relay     
                       REQUIRES esp_event nvs_flash esp_wifi mqtt  )
