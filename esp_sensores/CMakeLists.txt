cmake_minimum_required(VERSION 3.5)
set(EXTRA_COMPONENT_DIRS "mqtt_config" "sensor" "wifi_config" "json" "soil_moisture" "oled_display" "water_sensor" "relay")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp_sensores)