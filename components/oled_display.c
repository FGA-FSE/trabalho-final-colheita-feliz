#include <stdio.h>
#include "driver/i2c.h"
#include "ssd1306.h"
#include "esp_log.h"

// Definições de pinos e parâmetros para o I2C
#define I2C_MASTER_SCL_IO           22    // GPIO para SCL
#define I2C_MASTER_SDA_IO           21    // GPIO para SDA
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0

// Endereço I2C do display OLED
#define OLED_ADDR                   0x3C

static const char *TAG = "OLED";

// Instância do display OLED
static ssd1306_handle_t dev = NULL;

// Função para inicializar o driver I2C
void i2c_master_init() {
    // Configuração do I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    // Configurar os parâmetros do I2C
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    // Instalar o driver I2C
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
}

// Função para inicializar o display OLED
void oled_init() {
    // Inicializar o I2C antes de criar o display
    i2c_master_init();

    dev = ssd1306_create(I2C_MASTER_NUM, OLED_ADDR);
    if (dev == NULL) {
        ESP_LOGE(TAG, "Falha na criação do SSD1306");
        while (1); // Loop infinito para indicar falha
    }
    ssd1306_clear_screen(dev, 0x00);
    const char* message = "Soil Moisture Monitor";
    ssd1306_draw_string(dev, 0, 0, (const uint8_t*)message, 12, 1);
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
