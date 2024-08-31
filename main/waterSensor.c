#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "gpio_setup.h"
#include "adc_module.h"
#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "WATER_SENSOR_APP";
static esp_mqtt_client_handle_t client; 

void water_sensor_task(void *pvParameter) {
    char payload[100];
    while (1) {

        int x = analogRead(ADC_CHANNEL_6);

        int porcentagem = x/23

        sprintf(payload, "{\"waterLevel\": %d}", porcentagem);
        esp_mqtt_client_publish(client, "v1/devices/me/attributes", payload, 0, 1, 0);
        ESP_LOGI(TAG, "Published: %s", payload);
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Delay de 2 segundos
    }
}