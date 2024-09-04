#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "soil_moisture.h"
#include "oled_display.h"

#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_12
#define SENSOR_PIN ADC_CHANNEL_0

const static char *TAG = "SOIL_MOISTURE";

// Valores de referência para calibração da umidade do solo
const int AirValue = 3200;   // Valor de referência para o ar (seco)
const int WaterValue = 1199; // Valor de referência para a água (úmido)

// Variável para armazenar o handle do ADC
static adc_oneshot_unit_handle_t adc1_handle;

// Função para inicializar o sensor de umidade do solo
void init_soil_moisture(int pin) {
    // Inicializar o ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Configurar o canal ADC para o sensor de umidade do solo
    adc_oneshot_chan_cfg_t config = {
        .atten = EXAMPLE_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, pin, &config));
}

// Função para ler a umidade do solo e retornar a porcentagem
int read_soil_moisture() {
    int adc_raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, SENSOR_PIN, &adc_raw));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, SENSOR_PIN, adc_raw);

    // Calcular a porcentagem de umidade
    int moisture_percent = (AirValue - adc_raw) * 100 / (AirValue - WaterValue);
    if (moisture_percent > 100) {
        moisture_percent = 100;
    } else if (moisture_percent < 0) {
        moisture_percent = 0;
    }
    ESP_LOGI(TAG, "Soil Moisture Percent: %d%%", moisture_percent);

    return moisture_percent;
}

// Task que lê e exibe a umidade do solo no OLED
void soil_moisture_task(void *pvParameter) {
    init_soil_moisture(SENSOR_PIN);

    while (1) {
        int moisture_percent = read_soil_moisture();

        // Função para exibir a umidade no display OLED
        oled_display_soil_moisture(moisture_percent);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
