#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "water_sensor.h"

static const char *TAG = "WATER_SENSOR";

// Usa o mesmo handle do ADC1 já inicializado em outro lugar (por exemplo, para o sensor de umidade do solo)
extern adc_oneshot_unit_handle_t adc1_handle;

void init_water_level_sensor(void)  // Sem o parâmetro 'pin'
{
    // Configuração do canal de nível de água, sem inicializar o ADC1 novamente
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_0,  // Atenuação padrão
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config));  // Usar o canal 7 para o sensor de nível de água
}

int read_water_level()
{
    int adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &adc_raw));  // Leitura do ADC no canal 7
    ESP_LOGI(TAG, "ADC Raw Data: %d", adc_raw);

    // Definir nível crítico com base na leitura do ADC
    int critico = (adc_raw > 3800) ? 1 : 0;  // 1 = tanque cheio, 0 = encher tanque

    return critico;
}
