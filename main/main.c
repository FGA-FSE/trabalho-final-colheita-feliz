#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "dht11.h"
#include "waterSensor.h"
#include "mqtt_app.h"
#include "esp_sleep.h"

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
    mqtt_app_start();

    //  leitura do DHT11
    // xTaskCreate(&dht_task, "dht_task", 2048, NULL, 5, NULL);

    xTaskCreate(&water_sensor_task, "water_sensor_task", 2048, NULL, 5, NULL);

}
