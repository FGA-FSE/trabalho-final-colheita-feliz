idf_component_register(SRCS "mqtt.c"
                       INCLUDE_DIRS "."
                       REQUIRES mqtt esp_wifi relay)