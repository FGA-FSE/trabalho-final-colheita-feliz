#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "adc_module.h"
#include "mqtt.h"


static const char *TAG = "WATER_SENSOR_APP";
static esp_mqtt_client_handle_t client; 

void sendWaterLevel() {
    char payload[100];
    while (1) {
        int x = analogRead(ADC_CHANNEL_6);
        
        int critico = x > 3800;

        sprintf(payload, "{\"waterLevel\": %d}", critico);
        mqtt_send_message("v1/devices/me/attributes", payload);
        ESP_LOGI(TAG, "Published: %s", payload);
    }
}