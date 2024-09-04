cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS "mqtt_config" "relay" "wifi_config" )
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp_rele)
