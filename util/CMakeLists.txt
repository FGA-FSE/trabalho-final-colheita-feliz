idf_component_register(SRCS "adc_module.c" "gpio_setup.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_event mqtt)
