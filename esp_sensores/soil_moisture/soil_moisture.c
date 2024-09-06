#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "soil_moisture.h"

static const char *TAG = "SOIL_MOISTURE";

// Usa o mesmo handle do ADC1 já inicializado
extern adc_oneshot_unit_handle_t adc1_handle;

void init_soil_moisture(void)
{
    
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,  
        .bitwidth = ADC_BITWIDTH_DEFAULT,  
    };
    
    // Configurar o canal 0 para o sensor de umidade do solo
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
}



int read_soil_moisture(void)
{
    int adc_raw = 0;  // Variável para armazenar a leitura do ADC
    esp_err_t ret = adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_raw);  // Leitura do ADC

    // Verificar se a leitura foi bem-sucedida
    if (ret != ESP_OK) {
        ESP_LOGE("SOIL_MOISTURE", "Falha ao ler o ADC: %s", esp_err_to_name(ret));
        return -1;  // Retorna um valor de erro
    }

    ESP_LOGI("SOIL_MOISTURE", "ADC Raw Data: %d", adc_raw);

    
    int adc_min = 1200;  
    int adc_max = 3200;  

    // Verificar se o valor está fora dos limites esperados
    if (adc_raw < adc_min) {
        adc_raw = adc_min;  
    } else if (adc_raw > adc_max) {
        adc_raw = adc_max;  
    }

    int moisture_percent = ((adc_max - adc_raw) * 100) / (adc_max - adc_min);

    return moisture_percent;
}
