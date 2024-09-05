#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt.h"
#include "cJSON.h"
#include "driver/ledc.h"
#include <stdbool.h>  

#define TAG "MQTT"

// Variáveis globais
bool dht11_sensor_active = true;
bool water_level_sensor_active = true;
extern SemaphoreHandle_t mqttConnectionSemaphore;

// Função de ajuste do brilho do LED
extern void set_led_brightness(int brightness);

esp_mqtt_client_handle_t client;

// Função para log de erros
static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

// Função para manipular o recebimento de RPC do MQTT
void handle_mqtt_rpc_request(esp_mqtt_event_handle_t event) {
    ESP_LOGI(TAG, "Recebendo comando RPC..."); 
    
    cJSON *request = cJSON_Parse(event->data);
    if (request == NULL) {
        ESP_LOGE(TAG, "Falha ao interpretar o JSON do RPC");
        return;
    }

    cJSON *method = cJSON_GetObjectItem(request, "method");
    cJSON *params = cJSON_GetObjectItem(request, "params");

    if (method == NULL || params == NULL) {
        ESP_LOGE(TAG, "Formato de mensagem RPC inválido");
        cJSON_Delete(request);
        return;
    }

    cJSON *active = cJSON_GetObjectItem(params, "active");
    cJSON *brightness = cJSON_GetObjectItem(params, "brightness");

    if (strcmp(method->valuestring, "setDHT11Sensor") == 0) {
        if (active != NULL) {
            dht11_sensor_active = active->valueint;
            ESP_LOGI(TAG, "DHT11 %s", dht11_sensor_active ? "Ativado" : "Desativado");
        }
    }

    if (strcmp(method->valuestring, "setWaterLevelSensor") == 0) {
        if (active != NULL) {
            water_level_sensor_active = active->valueint;
            ESP_LOGI(TAG, "Sensor de nível de água %s", water_level_sensor_active ? "Ativado" : "Desativado");
        }
    }

    if (strcmp(method->valuestring, "setLedBrightness") == 0) {
        if (brightness != NULL) {
            int brightness_value = brightness->valueint;
            set_led_brightness(brightness_value);  // Ajustar o brilho do LED
            ESP_LOGI(TAG, "Brilho do LED definido para %d%%", brightness_value);
        }
    }

    cJSON_Delete(request);
}

// Função de callback para eventos MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Evento MQTT: base=%s, event_id=%d", base, (int) event_id);
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado");
            xSemaphoreGive(mqttConnectionSemaphore);
            esp_mqtt_client_subscribe(client, "v1/devices/me/rpc/request/+", 0);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Recebendo dados MQTT...");
            handle_mqtt_rpc_request(event);
            break;

        default:
            break;
    }
}

// Função para iniciar o MQTT
void mqtt_start() {
    const esp_mqtt_client_config_t mqtt_config = {
       .broker.address.uri = "mqtt://164.41.98.25",
        .credentials.username = "Has1xP8P9E9mFEN882eX"
    };
    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

// Função para enviar mensagens via MQTT
void mqtt_send_message(char *topic, char *message) {
    int message_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    ESP_LOGI(TAG, "Mensagem enviada, ID: %d", message_id);
}
