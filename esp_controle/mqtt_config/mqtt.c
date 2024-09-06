#include "mqtt_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_sleep.h"


static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;
static SemaphoreHandle_t mqtt_message_semaphore;  // Semáforo para aguardar mensagens

// Declaração do callback de eventos
static void mqtt_event_handler_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_start() {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://test.mosquitto.org",  
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, client);
    esp_mqtt_client_start(client);

    // Inicialize o semáforo
    mqtt_message_semaphore = xSemaphoreCreateBinary();
}

int mqtt_send_message(const char *topic, const char *message) {
    if (client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return -1;
    }

    int msg_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    if (msg_id == -1) {
        ESP_LOGE(TAG, "Failed to publish message");
    } else {
        ESP_LOGI(TAG, "Message published, msg_id=%d", msg_id);
    }

    return msg_id;
}

static void mqtt_event_handler_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado ao broker!");

            // Enviar a mensagem após a conexão ser estabelecida
            const char *topico = "esp/rele";
            const char *mensagem = "{\"state\":\"on\"}";
            mqtt_send_message(topico, mensagem);

            ESP_LOGI(TAG, "Comando enviado: Tópico='%s', Mensagem='%s'", topico, mensagem);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Mensagem publicada, msg_id=%d", event->msg_id);

            // Entrar em deep sleep após a publicação da mensagem
            ESP_LOGI(TAG, "Entrando em deep sleep...");
            esp_deep_sleep_start();
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT mensagem recebida, tópico: %.*s, mensagem: %.*s",
                     event->topic_len, event->topic, event->data_len, event->data);
            xSemaphoreGive(mqtt_message_semaphore);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT desconectado do broker");
            break;

        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT antes de conectar...");
            break;

        default:
            ESP_LOGI(TAG, "Other MQTT event id: %d", event->event_id);
            break;
    }
}


bool mqtt_wait_for_message() {
    if (xSemaphoreTake(mqtt_message_semaphore, portMAX_DELAY) == pdTRUE) {
        ESP_LOGI(TAG, "Message received, semaphore released");
        return true;  // Mensagem recebida com sucesso
    } else {
        ESP_LOGE(TAG, "Failed to wait for message");
        return false;  // Falha ao aguardar por mensagem
    }
}
