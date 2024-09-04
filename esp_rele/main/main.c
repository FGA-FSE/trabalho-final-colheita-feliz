#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "mqtt.h"
#include "relay.h"

#define RELAY_PIN 25  

void app_main(void) {
    // Inicializa NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializa o relé
    relay_init(RELAY_PIN);

    // Inicia o MQTT
    mqtt_start();
}
