#include "driver/gpio.h"
#include "esp_log.h"

static int relay_pin;

void relay_init(int pin) {
    relay_pin = pin;
    esp_rom_gpio_pad_select_gpio(relay_pin);
    gpio_set_direction(relay_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(relay_pin, 0); // Desligar a bomba inicialmente
}

void relay_turn_on(void) {
    gpio_set_level(relay_pin, 1); // Liga a bomba
    ESP_LOGI("RELAY", "Bomba ligada");
}

void relay_turn_off(void) {
    gpio_set_level(relay_pin, 0); // Desliga a bomba
    ESP_LOGI("RELAY", "Bomba desligada");
}
