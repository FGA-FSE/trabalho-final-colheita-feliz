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
#include "relay.h"  // Certifique-se de incluir relay.h para usar process_relay_command

#define TAG "MQTT"

// Variáveis globais
bool dht11_sensor_active = true;
bool water_level_sensor_active = true;
extern SemaphoreHandle_t mqttConnectionSemaphore;

esp_mqtt_client_handle_t client_thingsboard;   // Cliente para conexão com ThingsBoard
esp_mqtt_client_handle_t client_mosquitto;     // Cliente para conexão com Mosquitto

// Função de ajuste do brilho do LED verde
extern void set_led_brightness(int green_brightness);

// Função para log de erros
static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Último erro %s: 0x%x", message, error_code);
    }
}

/// Função para manipular o recebimento de RPC do MQTT (ThingsBoard)
void handle_mqtt_rpc_request(esp_mqtt_event_handle_t event) {
    ESP_LOGI(TAG, "Recebendo comando RPC...");

    // Verifica se o payload foi recebido corretamente
    ESP_LOGI(TAG, "Payload recebido: %.*s", event->data_len, event->data);

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

    // Processamento do comando para ajustar o brilho do LED verde
    if (strcmp(method->valuestring, "setLedBrightness") == 0) {
        int green_value = params->valueint;
        ESP_LOGI(TAG, "Brilho recebido para o LED verde: %d%%", green_value);
        set_led_brightness(green_value);  // Ajustar o brilho do LED verde
    }

    cJSON_Delete(request);
}

// Função de callback para eventos MQTT do ThingsBoard
static void mqtt_event_handler_thingsboard(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Evento MQTT: base=%s, event_id=%d", base, (int)event_id);
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado ao ThingsBoard");
            xSemaphoreGive(mqttConnectionSemaphore);
            esp_mqtt_client_subscribe(client_thingsboard, "v1/devices/me/rpc/request/+", 0);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Recebendo dados MQTT...");
            handle_mqtt_rpc_request(event);
            break;

        default:
            break;
    }
}

// Função de callback para eventos MQTT do Mosquitto
void mqtt_event_handler_mosquitto(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado ao Mosquitto!");
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT desconectado do Mosquitto!");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Recebendo dados via Mosquitto...");
            ESP_LOGI(TAG, "Tópico: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Dados: %.*s", event->data_len, event->data);

            // Converte o payload recebido para cJSON
            cJSON *request = cJSON_Parse(event->data);
            if (request == NULL) {
                ESP_LOGE(TAG, "Falha ao interpretar o JSON recebido");
                return;
            }

            // Passa o objeto JSON para o processamento
            ESP_LOGI(TAG, "JSON processado com sucesso. Enviando para o process_relay_command");
            process_relay_command(request);  // Verifique se essa função é chamada corretamente
            cJSON_Delete(request);  // Libera a memória do objeto JSON
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Ocorreu um erro com o MQTT!");
            break;

        default:
            ESP_LOGI(TAG, "Outro evento MQTT: %d", event->event_id);
            break;
    }
}


// Função para iniciar a conexão MQTT com o ThingsBoard
void mqtt_start_thingsboard() {
    const esp_mqtt_client_config_t mqtt_config_thingsboard = {
        .broker.address.uri = "mqtt://164.41.98.25", // Endereço do broker do ThingsBoard
        .credentials.username = "Has1xP8P9E9mFEN882eX"
    };
    client_thingsboard = esp_mqtt_client_init(&mqtt_config_thingsboard);
    esp_mqtt_client_register_event(client_thingsboard, MQTT_EVENT_ANY, mqtt_event_handler_thingsboard, NULL);
    esp_mqtt_client_start(client_thingsboard);
}

// Função para iniciar a conexão MQTT com o Mosquitto (para comunicação entre ESPs)
void mqtt_start_mosquitto() {
    ESP_LOGI(TAG, "Iniciando conexão com o Mosquitto...");
    const esp_mqtt_client_config_t mqtt_config_mosquitto = {
        .broker.address.uri = "mqtt://test.mosquitto.org",  // Endereço do broker Mosquitto
    };
    client_mosquitto = esp_mqtt_client_init(&mqtt_config_mosquitto);
    if (client_mosquitto == NULL) {
        ESP_LOGE(TAG, "Falha ao inicializar o cliente MQTT para Mosquitto!");
        return;
    }

    esp_mqtt_client_register_event(client_mosquitto, MQTT_EVENT_ANY, mqtt_event_handler_mosquitto, NULL);
    esp_mqtt_client_start(client_mosquitto);
}


// Função para enviar mensagens via MQTT para o ThingsBoard
void mqtt_send_message_thingsboard(const char *topic, const char *message) {
    int message_id = esp_mqtt_client_publish(client_thingsboard, topic, message, 0, 1, 0);
    ESP_LOGI(TAG, "Mensagem enviada para o ThingsBoard, ID: %d", message_id);
}

// Função para enviar mensagens via MQTT para o Mosquitto
void mqtt_send_message_mosquitto(const char *topic, const char *message) {
    int message_id = esp_mqtt_client_publish(client_mosquitto, topic, message, 0, 1, 0);
    ESP_LOGI(TAG, "Mensagem enviada para o Mosquitto, ID: %d", message_id);
}
