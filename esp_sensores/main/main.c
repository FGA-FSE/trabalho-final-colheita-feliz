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
void init_pwm_led(int gpio_num, ledc_channel_t channel)
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
        .channel        = channel,  // Canal passado como parâmetro
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,  // Duty inicial 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// Função para ajustar a cor dos LEDs com base na umidade do ar
void set_led_color_by_humidity(int humidity)
{
    int duty_max = (1 << LEDC_TIMER_13_BIT) - 1;  // Valor máximo de duty cycle (8191)
    int red_duty = 0;
    int green_duty = 0;

    // Quanto menor a umidade, mais vermelho o LED será
    if (humidity < 10) {
        red_duty = duty_max;  // Máximo vermelho
        green_duty = 0;       // Verde apagado
    } else if (humidity >= 10 && humidity <= 70) {
        // Interpolação de cores entre vermelho e verde
        red_duty = (70 - humidity) * duty_max / 40;  // O vermelho diminui conforme a umidade aumenta
        green_duty = (humidity - 30) * duty_max / 40;  // O verde aumenta conforme a umidade aumenta
    } else {
        red_duty = 0;         // Apagar o vermelho
        green_duty = duty_max; // Máximo verde
    }

    // Aplicar os valores de duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, red_duty));  // LED vermelho
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0));
    
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, green_duty)); // LED verde
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1));
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

int last_valid_temperature = -1;  // Armazenar a última leitura válida
int last_valid_humidity = -1;

void sendSensorDataToDashboard()
{
    char message[256];
    struct dht11_reading dht11_value;

    ESP_LOGI("DHT11", "Tentando ler sensor...");

    // Ler valores do DHT11
    dht11_value = DHT11_read();

    // Definir limites aceitáveis para a temperatura e umidade
    int temp_min = 0, temp_max = 50;
    int hum_min = 0, hum_max = 100;

    // Verificar se a leitura está dentro dos limites aceitáveis
    if (dht11_value.temperature >= temp_min && dht11_value.temperature <= temp_max &&
        dht11_value.humidity >= hum_min && dht11_value.humidity <= hum_max)
    {
        last_valid_temperature = dht11_value.temperature;
        last_valid_humidity = dht11_value.humidity;
    }
    else
    {
        // Se a leitura for inválida, usar a última leitura válida
        dht11_value.temperature = last_valid_temperature;
        dht11_value.humidity = last_valid_humidity;
        ESP_LOGW("DHT11", "Leitura inválida, usando último valor válido.");
    }

    // Ler o solo e nível de água como de costume
    int soil_moisture = read_soil_moisture();
    int water_level = read_water_level();

    // Cria a mensagem JSON com os dados
    sprintf(message, "{\"temperature\": %d, \"humidity\": %d, \"soil_moisture\": %d, \"waterLevel\": %d}",
            dht11_value.temperature, dht11_value.humidity, soil_moisture, water_level);

    // Enviar via MQTT
    mqtt_send_message("v1/devices/me/telemetry", message);

    ESP_LOGI("DHT11", "Temperature: %d, Humidity: %d, Soil Moisture: %d, Water Level: %d",
             dht11_value.temperature, dht11_value.humidity, soil_moisture, water_level);

    // Atualizar OLED
    oled_display_update(dht11_value.temperature, dht11_value.humidity, soil_moisture);

    // Ajustar cor dos LEDs com base na umidade do ar
    set_led_color_by_humidity(dht11_value.humidity);
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
    init_soil_moisture();

    // Inicializar o sensor de nível de água
    init_water_level_sensor();

    // Inicializar LEDs PWM para o controle de cor
    init_pwm_led(LED_RED_GPIO, LEDC_CHANNEL_0);   // LED vermelho no canal 0
    init_pwm_led(LED_GREEN_GPIO, LEDC_CHANNEL_1); // LED verde no canal 1

    // Criar as tarefas
    xTaskCreate(&wifiConnected, "Wi-Fi Connection", 4096, NULL, 2, NULL);  
    xTaskCreate(&handleServerCommunication, "Server Communication", 4096, NULL, 1, NULL);
}
