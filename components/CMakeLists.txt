# Registro de componentes para o projeto ESP-IDF sem dependências do Arduino

idf_component_register(SRCS "soil_moisture.c" "mqtt_app.c" "oled_display.c" "rele.c" "gpio_setup.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_event mqtt driver ssd1306 esp_adc)