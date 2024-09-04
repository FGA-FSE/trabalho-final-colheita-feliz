#include <stdio.h>
#include <string.h>
#include "mqtt.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "relay.h"

static const char *TAG = "MQTT";

// Callback para lidar com os eventos MQTT
void mqtt_event_handler_cb(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_DATA: {
            char topic[event->topic_len + 1];
            char message[event->data_len + 1];

            strncpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = '\0';
            strncpy(message, event->data, event->data_len);
            message[event->data_len] = '\0';

            ESP_LOGI(TAG, "Topic: %s, Message: %s", topic, message);

            // Tratar comando recebido
            if (strcmp(message, "water_pump_on") == 0) {
                relay_turn_on(); // Liga a bomba de água
            } else if (strcmp(message, "water_pump_off") == 0) {
                relay_turn_off(); // Desliga a bomba de água
            }
            break;
        }

        default:
            ESP_LOGI(TAG, "Evento MQTT não tratado: %d", event->event_id);
            break;
    }
}

// Função para iniciar o MQTT
void mqtt_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://164.41.98.25",
        .credentials.username = "Has1xP8P9E9mFEN882eX"
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, NULL);
    esp_mqtt_client_start(client);
}
