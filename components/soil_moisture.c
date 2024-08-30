#include <stdio.h>
#include <string.h> // Inclua para memset
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "soil_moisture.h"
#include "esp_adc/adc_oneshot.h"

// Definições de pinos e parâmetros para o I2C
#define I2C_MASTER_SCL_IO           22    // GPIO para SCL
#define I2C_MASTER_SDA_IO           21    // GPIO para SDA
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

// Endereço I2C do display OLED
#define OLED_ADDR                   0x3C

// Definições para o sensor de umidade do solo
const int AirValue = 3620;
const int WaterValue = 1680;
const int SensorPin = ADC1_CHANNEL_6;  // GPIO34 no ESP32

// Criação de uma instância do display
ssd1306_handle_t dev = NULL;

// Função para inicializar o display OLED
void oled_init() {
    dev = ssd1306_create(I2C_MASTER_NUM, OLED_ADDR);
    if (dev == NULL) {
        ESP_LOGE("OLED", "Falha na criação do SSD1306");
        while (1); // Loop infinito para indicar falha
    }
    ssd1306_clear_screen(dev, 0x00);
    const char* message = "Soil Moisture Monitor";
    ssd1306_draw_string(dev, 0, 0, (const uint8_t*)message, 12, 1);
    ssd1306_refresh_gram(dev); // Atualiza o display para mostrar as mudanças
}

// Função para exibir a umidade no display OLED
void oled_display_soil_moisture(int soilmoisturepercent) {
    ssd1306_clear_screen(dev, 0x00);
    char buffer[20];
    sprintf(buffer, "Soil: %d%%", soilmoisturepercent);
    ssd1306_draw_string(dev, 0, 0, (const uint8_t*)buffer, 12, 1);
    ssd1306_refresh_gram(dev); // Atualiza o display
}

// Função de tarefa para monitoramento de umidade do solo
void soil_moisture_task(void *pvParameter) {
    // Inicialização do display OLED
    oled_init();

    // Configurações do ADC para o sensor de umidade
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    adc_oneshot_config_channel(adc1_handle, SensorPin, &config);

    while (1) {
        // Leitura do valor do sensor de umidade
        int soilMoistureValue;
        adc_oneshot_read(adc1_handle, SensorPin, &soilMoistureValue);
        printf("Soil Moisture Value: %d\n", soilMoistureValue);

        // Conversão para porcentagem
        int soilmoisturepercent = (soilMoistureValue - WaterValue) * 100 / (AirValue - WaterValue);
        if (soilmoisturepercent > 100) {
            soilmoisturepercent = 100;
        } else if (soilmoisturepercent < 0) {
            soilmoisturepercent = 0;
        }

        // Exibir no console e no display OLED
        printf("Soil Moisture Percent: %d%%\n", soilmoisturepercent);
        oled_display_soil_moisture(soilmoisturepercent);

        vTaskDelay(pdMS_TO_TICKS(1000));  // Atraso de 1 segundo
    }
}