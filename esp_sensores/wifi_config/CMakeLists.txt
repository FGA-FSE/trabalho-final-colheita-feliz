idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "."
                    REQUIRES freertos esp_wifi esp_system esp_event nvs_flash lwip main mqtt_config)
