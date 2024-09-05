#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "oled_display.h"
#include "dht11.h"
#include "soil_moisture.h"
#include "driver/adc.h"
#include "ssd1306.h"  // Certifique-se de que esta inclusão esteja correta para o seu ambiente

// Definições de pinos e parâmetros para o I2C
#define I2C_MASTER_SCL_IO           15    // GPIO para SCL (D15)
#define I2C_MASTER_SDA_IO           4     // GPIO para SDA (D4)
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

// Endereço I2C do display OLED
#define OLED_ADDR                   0x3C

// Definições de pinos para sensores
#define DHT11_PIN                   18    // GPIO para DHT11 (D18)
#define SOIL_MOISTURE_PIN           ADC1_CHANNEL_0  // ADC1 Channel para o sensor de umidade do solo (GPIO36)

static const char *TAG = "OLED";

// Instância global do display OLED
static ssd1306_handle_t dev = NULL;

// Função para inicializar o driver I2C
void i2c_master_init(int sda_pin, int scl_pin) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl_pin,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
}

// Função para inicializar o display OLED
void oled_display_init(int sda_pin, int scl_pin) {
    // Inicializar o I2C com os pinos especificados
    i2c_master_init(sda_pin, scl_pin);

    dev = ssd1306_create(I2C_MASTER_NUM, OLED_ADDR);
    if (dev == NULL) {
        ESP_LOGE(TAG, "Falha na criação do SSD1306");
        while (1); // Loop infinito para indicar falha
    }
    ssd1306_clear_screen(dev, 0x00);
    ssd1306_refresh_gram(dev); // Atualiza o display para mostrar as mudanças
    ESP_LOGI(TAG, "Display OLED inicializado com sucesso");
}

// Função para exibir a umidade no display OLED
void oled_display_soil_moisture(int soilmoisturepercent) {
    ssd1306_clear_screen(dev, 0x00);
    char buffer[20];
    sprintf(buffer, "Soil: %d%%", soilmoisturepercent);
    ssd1306_draw_string(dev, 0, 0, (const uint8_t*)buffer, 12, 1);
    ssd1306_refresh_gram(dev); // Atualiza o display
    ESP_LOGI(TAG, "Umidade do solo exibida: %d%%", soilmoisturepercent);
}

// Função para exibir múltiplos valores no display OLED
void oled_display_update(int temperature, int humidity, int soil_moisture) {
    ssd1306_clear_screen(dev, 0x00);

    char buffer[20];
    
    // Exibe a temperatura
    sprintf(buffer, "Temp: %d C", temperature);
    ssd1306_draw_string(dev, 0, 0, (const uint8_t*)buffer, 12, 1);

    // Exibe a umidade do ar
    sprintf(buffer, "Hum: %d%%", humidity);
    ssd1306_draw_string(dev, 0, 16, (const uint8_t*)buffer, 12, 1);

    // Exibe a umidade do solo
    sprintf(buffer, "Soil: %d%%", soil_moisture);
    ssd1306_draw_string(dev, 0, 32, (const uint8_t*)buffer, 12, 1);

    ssd1306_refresh_gram(dev); // Atualiza o display para mostrar as mudanças
    ESP_LOGI(TAG, "Display atualizado: Temp=%dC, Hum=%d%%, Soil=%d%%", temperature, humidity, soil_moisture);
}

