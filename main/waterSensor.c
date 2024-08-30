#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"

#include "gpio_setup.h"
#include "adc_module.h"

#include "esp_log.h"



int readSensor(){

    adc_init(ADC_UNIT_1);

    pinMode(ADC_CHANNEL_6, GPIO_ANALOG);

    do{

    int x = analogRead(ADC_CHANNEL_6);

    ESP_LOGI("ADC", "Raw ADC value from GPIO15: %d", x);
    
    
    vTaskDelay(500 / portTICK_PERIOD_MS);
        vTaskDelay(500/portTICK_PERIOD_MS);
    } while (1);

    return 0;
}