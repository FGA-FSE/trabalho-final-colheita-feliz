#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "water_sensor.h"

static const char *TAG = "WATER_SENSOR";


extern adc_oneshot_unit_handle_t adc1_handle;

void init_water_level_sensor(void)  
{
   
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_0,  
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config));  
}

int read_water_level()
{
    int adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &adc_raw));  // Leitura do ADC no canal 7
    ESP_LOGI(TAG, "ADC Raw Data: %d", adc_raw);

    int critico = (adc_raw > 3800) ? 1 : 0;  

    return critico;
}
