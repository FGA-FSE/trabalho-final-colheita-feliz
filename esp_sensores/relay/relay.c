#include "relay.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include <string.h>

#define TAG "RELAY"

// Função para inicializar o GPIO do relé
void init_relay(void) {
    esp_rom_gpio_pad_select_gpio(RELAY_GPIO);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_GPIO, 0);  // Inicialmente desligado
    ESP_LOGI(TAG, "Relé inicializado no GPIO %d", RELAY_GPIO);
}

// Função para definir o estado do relé (ligado/desligado)
void set_relay_state(bool state) {
    gpio_set_level(RELAY_GPIO, state ? 1 : 0);  
    ESP_LOGI(TAG, "Relé %s", state ? "Ligado" : "Desligado");
}

// Função para processar o comando do relé via JSON
void process_relay_command(cJSON *request) {
    cJSON *params = cJSON_GetObjectItem(request, "params");
    if (params == NULL) {
        ESP_LOGE(TAG, "Campo 'params' não encontrado no JSON");
        return;
    }

    ESP_LOGI(TAG, "Campo 'params' encontrado no JSON");

    cJSON *state = cJSON_GetObjectItem(params, "state");
    if (state == NULL) {
        ESP_LOGE(TAG, "Campo 'state' não encontrado dentro de 'params'");
        return;
    }

    ESP_LOGI(TAG, "Campo 'state' encontrado: %s", state->valuestring);


    if (cJSON_IsString(state) && (strcmp(state->valuestring, "on") == 0)) {
        ESP_LOGI(TAG, "Comando recebido: Ligando o relé...");
        set_relay_state(true);  

        ESP_LOGI(TAG, "Relé está ligado. Aguardando 5 segundos antes de desligar...");
        vTaskDelay(5000 / portTICK_PERIOD_MS); 

        ESP_LOGI(TAG, "Desligando o relé após 5 segundos...");
        set_relay_state(false);  
    } else {
        ESP_LOGE(TAG, "Comando inválido ou estado não reconhecido: %s", state->valuestring);
    }
}
