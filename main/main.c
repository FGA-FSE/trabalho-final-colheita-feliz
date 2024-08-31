#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "dht11.h"
#include "mqtt_app.h"
#include "soil_moisture.c" 

void app_main(void)
{
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializa o cliente MQTT
    // mqtt_app_start();

    //  leitura do DHT11
    // xTaskCreate(&dht_task, "dht_task", 2048, NULL, 5, NULL);

    // Tarefa de monitoramento de umidade do solo
     xTaskCreate(&soil_moisture_task, "soil_moisture_task", 4096, NULL, 5, NULL);

}
