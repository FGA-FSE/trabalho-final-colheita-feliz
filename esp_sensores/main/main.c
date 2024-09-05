#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "oled_display.h"
#include "wifi.h"
#include "mqtt.h"
#include "dht11.h"
#include "soil_moisture.h"  // Inclui o sensor de umidade do solo
#include "water_sensor.h"    // Inclui o sensor de nível de água
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"


static const char *TAG = "MAIN"; 
adc_oneshot_unit_handle_t adc1_handle;

// Definição dos pinos
#define DHT11_PIN 18
#define LED_GREEN_GPIO 23
#define LED_RED_GPIO 21
#define LED_BLUE_GPIO 19
#define SOIL_MOISTURE_PIN ADC1_CHANNEL_0   // Pino ADC para o sensor de umidade do solo (GPIO 36 - canal 0 do ADC1)
#define WATER_SENSOR_PIN ADC1_CHANNEL_7    // Pino ADC para o sensor de nível de água (GPIO 35 - canal 7 do ADC1)
#define OLED_SDA_PIN 4                     // Pino SDA do display OLED
#define OLED_SCL_PIN 15                    // Pino SCL do display OLED

SemaphoreHandle_t wifiConnectionSemaphore;
SemaphoreHandle_t mqttConnectionSemaphore;

// Função para inicializar o PWM no GPIO
void init_pwm_led(int gpio_num)
{
    // Configurar o temporizador LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = 5000,  // Frequência de 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configurar o canal LEDC
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = gpio_num,
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,  // Duty inicial 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// Função para ajustar o brilho do LED com base no duty cycle
void set_led_brightness(int soil_moisture)
{
    int duty_max = (1 << LEDC_TIMER_13_BIT) - 1;  // Valor máximo de duty cycle (8191)
    int duty = 0;  // Duty inicial para o LED (apagado)

    // Definir brilho com base nos níveis de umidade do solo
    if (soil_moisture <= 5) {
        duty = 0;  // Entre 0% e 5%, LED apagado
    } else if (soil_moisture > 5 && soil_moisture <= 10) {
        duty = (10 * duty_max) / 100;  // Entre 5% e 10%, LED com luz fraca (10% do brilho)
    } else {
        // Acima de 10%, ajustar o brilho proporcionalmente
        duty = (soil_moisture * duty_max) / 100;  // Ajusta brilho com base na umidade
    }

    // Aplicar o duty cycle ao LED
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0));
}

void init_adc1(void)
{
    // Configuração da unidade ADC1
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,  // Usar ADC1
    };

    // Inicializar o ADC1 e armazenar o handle em adc1_handle
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
}


void wifiConnected(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(wifiConnectionSemaphore, portMAX_DELAY))
        {
            mqtt_start();  // Iniciar MQTT após a conexão Wi-Fi
        }
    }
}

void sendSensorDataToDashboard()
{
    char message[256];
    struct dht11_reading dht11_value;

    ESP_LOGI("DHT11", "Tentando ler sensor...");

    // Ler valores do DHT11
    dht11_value = DHT11_read();

    // Ler valor da umidade do solo
    int soil_moisture = read_soil_moisture();  // Chama a função para ler a umidade do solo

    // Ler valor do nível de água
    int water_level = read_water_level();  // Chama a função para ler o nível de água

    if (dht11_value.status == DHT11_OK)
    {
        // Cria a mensagem JSON com temperatura, umidade, umidade do solo e nível de água
        sprintf(message, "{\"temperature\": %d, \"humidity\": %d, \"soil_moisture\": %d, \"waterLevel\": %d}", 
                dht11_value.temperature, dht11_value.humidity, soil_moisture, water_level);
        
        mqtt_send_message("v1/devices/me/telemetry", message);

        ESP_LOGI("DHT11", "Temperature: %d, Humidity: %d, Soil Moisture: %d, Water Level: %d", 
                 dht11_value.temperature, dht11_value.humidity, soil_moisture, water_level);

        // Atualizar o display OLED com a temperatura, umidade e umidade do solo
        oled_display_update(dht11_value.temperature, dht11_value.humidity, soil_moisture);

        // Ajustar o brilho do LED com base na umidade do solo
        set_led_brightness(soil_moisture);  // Usar a leitura de umidade para o brilho do LED
    }
    else
    {
        ESP_LOGE("DHT11", "Falha ao ler o sensor DHT11. Status: %d", dht11_value.status);
    }
}

void handleServerCommunication(void *params)
{
    if (xSemaphoreTake(mqttConnectionSemaphore, portMAX_DELAY))
    {
        while (true)
        {
            vTaskDelay(10000 / portTICK_PERIOD_MS);  // Atraso de 10 segundos
            sendSensorDataToDashboard();
        }
    }
}

void app_main(void)
{
    // Inicializa a NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifiConnectionSemaphore = xSemaphoreCreateBinary();
    mqttConnectionSemaphore = xSemaphoreCreateBinary();
    
    ESP_LOGI(TAG, "Iniciando Wi-Fi...");
    wifi_start();  // Inicia o Wi-Fi

    // Inicializar o ADC1
    init_adc1();


    // Inicializar DHT11
    DHT11_init(DHT11_PIN);  

    // Inicializar o display OLED
    oled_display_init(OLED_SDA_PIN, OLED_SCL_PIN);

        // Inicializar o sensor de umidade do solo
    init_soil_moisture();  // Passar o pino do sensor de umidade do solo

    // Inicializar o sensor de nível de água
    init_water_level_sensor();  // Passar o pino do sensor de nível de água


    // Inicializar o LED PWM
    init_pwm_led(LED_GREEN_GPIO);

    // Criar as tarefas
    xTaskCreate(&wifiConnected, "Wi-Fi Connection", 4096, NULL, 2, NULL);  
    xTaskCreate(&handleServerCommunication, "Server Communication", 4096, NULL, 1, NULL);
}
