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
    gpio_set_level(RELAY_GPIO, state ? 1 : 0);  // 1 para ligar, 0 para desligar
    ESP_LOGI(TAG, "Relé %s", state ? "Ligado" : "Desligado");
}

// Função para processar o comando do relé
void process_relay_command(cJSON *request) {
    cJSON *state = cJSON_GetObjectItem(request, "state");

    if (cJSON_IsString(state) && (strcmp(state->valuestring, "on") == 0)) {
        ESP_LOGI(TAG, "Comando recebido: Ligando o relé...");
        set_relay_state(true);  // Liga o relé
        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Espera por 5 segundos
        ESP_LOGI(TAG, "Desligando o relé após 5 segundos...");
        set_relay_state(false);  // Desliga o relé
    } else {
        ESP_LOGE(TAG, "Comando inválido ou estado não reconhecido no JSON");
    }
}
