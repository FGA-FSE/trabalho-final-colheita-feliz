#include "button.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "mqtt.h"
#include "wifi.h"
#include "nvs_flash.h"

#define TAG "ESP_CONTROLE"
#define BUTTON_GPIO 0  // GPIO 0 conectado ao botão

SemaphoreHandle_t wifiConnectionSemaphore;

void app_main(void) {
    // Inicializa a NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializa o semáforo para o Wi-Fi
    wifiConnectionSemaphore = xSemaphoreCreateBinary();

    // Configura o deep sleep para acordar quando o GPIO 0 for pressionado (nível baixo)
    esp_sleep_enable_ext0_wakeup(BUTTON_GPIO, 0);

    // Inicializa o Wi-Fi
    wifi_start();

    // Espera até a conexão Wi-Fi estar estabelecida
    if (xSemaphoreTake(wifiConnectionSemaphore, portMAX_DELAY)) {
        ESP_LOGI(TAG, "Wi-Fi conectado!");

        // Inicializa o MQTT após a conexão Wi-Fi
        mqtt_start();

        // Espera pela mensagem MQTT ser enviada
        if (mqtt_wait_for_message()) {
            ESP_LOGI(TAG, "Mensagem MQTT enviada com sucesso");
        }

        // Entrar em deep sleep após a mensagem ser enviada
        ESP_LOGI(TAG, "Entrando em deep sleep...");
        esp_deep_sleep_start();
    } else {
        ESP_LOGE(TAG, "Falha ao conectar ao Wi-Fi.");
    }
}
