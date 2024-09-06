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
    // Configuração do canal de umidade do solo (canal 0)
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,  // Usar atenuação de 11 dB para maior faixa de leitura de tensão
        .bitwidth = ADC_BITWIDTH_DEFAULT,  // Largura de bit padrão (12 bits)
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

    // Definir os valores máximos e mínimos de umidade com base nos testes
    int adc_min = 1200;  // Valor mínimo de leitura do sensor quando o solo está seco
    int adc_max = 3200;  // Valor máximo de leitura do sensor quando o solo está totalmente molhado

    // Verificar se o valor está fora dos limites esperados
    if (adc_raw < adc_min) {
        adc_raw = adc_min;  // Evitar valores negativos
    } else if (adc_raw > adc_max) {
        adc_raw = adc_max;  // Limitar o valor máximo
    }

    // Calcular a porcentagem de umidade do solo
    int moisture_percent = ((adc_max - adc_raw) * 100) / (adc_max - adc_min);

    return moisture_percent;
}
