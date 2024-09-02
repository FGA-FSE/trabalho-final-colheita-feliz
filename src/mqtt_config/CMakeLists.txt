idf_component_register(SRCS "mqtt.c"
                       INCLUDE_DIRS "."
                       REQUIRES mqtt esp_event nvs_flash esp_wifi json sensor)
