idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES mqtt_config sensor wifi_config json      
                       REQUIRES esp_event nvs_flash esp_wifi mqtt)