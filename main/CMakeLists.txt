idf_component_register(SRCS "main.c" "dht11.c" "wifi_ap.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES driver esp_wifi esp_http_server nvs_flash components )
