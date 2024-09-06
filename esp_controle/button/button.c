#include "button.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"

#define BUTTON_GPIO 0  

void init_button(void) {
    gpio_reset_pin(BUTTON_GPIO);  
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON_GPIO);  // Habilitar pull-down para garantir o estado baixo
    ESP_LOGI("BUTTON", "Botão inicializado no GPIO %d", BUTTON_GPIO);
}

void init_deep_sleep(void) {
    esp_sleep_enable_ext0_wakeup(BUTTON_GPIO, 0);  // Acordar com o nível lógico 0
    ESP_LOGI("SLEEP", "Entrando em deep sleep...");
    esp_deep_sleep_start();  // Iniciar o deep sleep sem limite de tempo
}
