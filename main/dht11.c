#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_rom_sys.h"  
#define DHT11_PIN GPIO_NUM_4
static const char *TAG = "DHT11_APP";
static esp_mqtt_client_handle_t client; 
int readDHT11(int *humidity, int *temperature) {
    int result = 0;

    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS); 
    gpio_set_level(DHT11_PIN, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);

    while (gpio_get_level(DHT11_PIN) == 1) {}
  
    while (gpio_get_level(DHT11_PIN) == 0) {}
    *humidity = 50;      // Valor lido do DHT11
    *temperature = 25;   // Valor lido do DHT11
    return result;
}
void dht_task(void *pvParameter) {
    char payload[100];
    int humidity = 0, temperature = 0;
    while (1) {
        if (readDHT11(&humidity, &temperature) == 0) {
            sprintf(payload, "{\"temperature\": %d, \"humidity\": %d}", temperature, humidity);
            esp_mqtt_client_publish(client, "v1/devices/me/telemetry", payload, 0, 1, 0);
            ESP_LOGI(TAG, "Published: %s", payload);
        } else {
            ESP_LOGE(TAG, "Failed to read from DHT11 sensor");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay de 2 segundos
    }
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}
// void app_main(void) {
//     // Inicializar o cliente MQTT e outras configurações
//     const esp_mqtt_client_config_t mqtt_cfg = {
//         .broker.address.uri = "mqtt://164.41.98.25", // Substitua pelo URI do seu broker MQTT
//     };
//     client = esp_mqtt_client_init(&mqtt_cfg);
//     esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
//     esp_mqtt_client_start(client);
//     // Cria a tarefa DHT11
//     xTaskCreate(&dht_task, "dht_task", 2048, NULL, 5, NULL);
// }














