#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "oled_display.h"
#include "wifi.h"
#include "mqtt.h"
#include "dht11.h"
#include "soil_moisture.h"
#include "water_sensor.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "relay.h"

#define TAG "MAIN"

// Definição dos pinos
#define DHT11_PIN 18
#define LED_GREEN_GPIO 23
#define SOIL_MOISTURE_PIN ADC1_CHANNEL_0   // Pino ADC para o sensor de umidade do solo (GPIO 36 - canal 0 do ADC1)
#define WATER_SENSOR_PIN ADC1_CHANNEL_7    // Pino ADC para o sensor de nível de água (GPIO 35 - canal 7 do ADC1)
#define OLED_SDA_PIN 4                     // Pino SDA do display OLED
#define OLED_SCL_PIN 15                    // Pino SCL do display OLED
#define RELAY_GPIO 25                      // GPIO 25 para o controle do módulo relé

adc_oneshot_unit_handle_t adc1_handle;
extern bool dht11_sensor_active;  
extern bool water_level_sensor_active;  

SemaphoreHandle_t wifiConnectionSemaphore;
SemaphoreHandle_t mqttConnectionSemaphore;

// Função para inicializar o PWM para o LED verde
void init_pwm_led(int gpio_num, ledc_channel_t channel) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,  
        .freq_hz          = 5000,  
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = gpio_num,
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = channel,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,  
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// Função para ajustar o brilho do LED verde
void set_led_brightness(int green_brightness) {
    int green_duty = (green_brightness * 8191) / 100;  

    ESP_LOGI(TAG, "Ajustando o duty cycle do LED verde para: %d (Brilho: %d%%)", green_duty, green_brightness);

    // Definir o duty cycle para o LED verde
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, green_duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1));
}

// Função para inicializar o ADC1
void init_adc1(void) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,  
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
}

// Função para enviar dados dos sensores para o MQTT
void sendSensorDataToDashboard() {
    char message[256]; 
    int temperature = -1;
    int humidity = -1;
    int water_level = -1;

    // Lendo o sensor de umidade do solo
    int soil_moisture = read_soil_moisture();

   
    if (dht11_sensor_active) {
        temperature = DHT11_read().temperature;
        humidity = DHT11_read().humidity;
        ESP_LOGI("SENSORS", "Temp: %d, Hum: %d", temperature, humidity);
    } else {
        ESP_LOGI("SENSORS", "DHT11 desativado, ignorando leitura.");
    }

 
    if (water_level_sensor_active) {
        water_level = read_water_level();
        ESP_LOGI("SENSORS", "Water Level: %d", water_level);
    } else {
        ESP_LOGI("SENSORS", "Sensor de nível de água desativado, ignorando leitura.");
    }

    // Atualizando o display OLED
    oled_display_update(temperature, humidity, soil_moisture);


    snprintf(message, sizeof(message), "{\"temperature\":%d,\"humidity\":%d,\"soil_moisture\":%d,\"water_level\":%d}", 
             temperature, humidity, soil_moisture, water_level);

    // Enviando a mensagem via MQTT
    mqtt_send_message_thingsboard("v1/devices/me/telemetry", message);
}

// Tarefa de comunicação com o servidor MQTT
void handleServerCommunication(void *params) {
    if (xSemaphoreTake(mqttConnectionSemaphore, portMAX_DELAY)) {
        while (true) {
            vTaskDelay(10000 / portTICK_PERIOD_MS);  
            sendSensorDataToDashboard();  
        }
    }
}

void app_main(void) {
    // Inicializa a NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Criação dos semáforos
    wifiConnectionSemaphore = xSemaphoreCreateBinary();
    mqttConnectionSemaphore = xSemaphoreCreateBinary();

    // Inicializa Wi-Fi
    ESP_LOGI(TAG, "Iniciando Wi-Fi...");
    wifi_start();

    // Inicializar os sensores e periféricos
    init_pwm_led(LED_GREEN_GPIO, LEDC_CHANNEL_1);
    set_led_brightness(50);  

    init_adc1();
    DHT11_init(DHT11_PIN);
    oled_display_init(OLED_SDA_PIN, OLED_SCL_PIN);
    init_soil_moisture();
    init_water_level_sensor();
    init_relay();
 
    set_relay_state(false); 

    // Iniciar o MQTT para ThingsBoard e Mosquitto
    mqtt_start_thingsboard();
    mqtt_start_mosquitto();

    // Criar tarefas de comunicação
    xTaskCreate(&handleServerCommunication, "Server Communication", 4096, NULL, 1, NULL);
}
