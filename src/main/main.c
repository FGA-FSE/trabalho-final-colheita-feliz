#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "oled_display.h"
#include "wifi.h"
#include "mqtt.h"
#include "dht11.h"
#include "soil_moisture.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DHT11_PIN 18
#define LED_GREEN_GPIO 23
#define LED_RED_GPIO 21
#define LED_BLUE_GPIO 19  

SemaphoreHandle_t wifiConnectionSemaphore;
SemaphoreHandle_t mqttConnectionSemaphore;

void wifiConnected(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(wifiConnectionSemaphore, portMAX_DELAY))
        {
            // Start MQTT after Wi-Fi connection
            mqtt_start();
        }
    }
}

void sendSensorDataToDashboard()
{
    char message[256];
    struct dht11_reading dht11_value;

    ESP_LOGI("DHT11", "Attempting to read sensor...");

    dht11_value = DHT11_read();

    if (dht11_value.status == DHT11_OK)
    {
        sprintf(message, "{\"temperature\": %d, \"humidity\": %d}", 
                dht11_value.temperature, dht11_value.humidity);
        mqtt_send_message("v1/devices/me/telemetry", message);

        ESP_LOGI("DHT11", "Temperature: %d, Humidity: %d", 
                 dht11_value.temperature, dht11_value.humidity);

        // Update OLED display
        oled_display_update(dht11_value.temperature, dht11_value.humidity, -1); // -1 as a placeholder for soil moisture

        // Determine LED status based on temperature and humidity
        if (dht11_value.temperature >= 18 && dht11_value.temperature <= 26 &&
            dht11_value.humidity >= 40 && dht11_value.humidity <= 50)
        {
            ESP_LOGI("LED", "Conditions good. Turning on GREEN LED.");
            // Conditions are good: Turn on green LED, turn off others
            gpio_set_level(LED_GREEN_GPIO, 1);
            gpio_set_level(LED_RED_GPIO, 0);
            gpio_set_level(LED_BLUE_GPIO, 0);
        }
        else
        {
            ESP_LOGI("LED", "Conditions bad. Turning on RED LED.");
            // Conditions are not good: Turn on red LED, turn off others
            gpio_set_level(LED_GREEN_GPIO, 0);
            gpio_set_level(LED_RED_GPIO, 1);
            gpio_set_level(LED_BLUE_GPIO, 0);
        }
    }
    else
    {
        ESP_LOGE("DHT11", "Failed to read from DHT11 sensor. Status: %d", dht11_value.status);
        ESP_LOGI("LED", "Sensor read error. Turning on BLUE LED.");
        // In case of an error, turn on the blue LED 
        gpio_set_level(LED_GREEN_GPIO, 0);
        gpio_set_level(LED_RED_GPIO, 0);
        gpio_set_level(LED_BLUE_GPIO, 1);  
    }
}

void handleServerCommunication(void *params)
{
    if (xSemaphoreTake(mqttConnectionSemaphore, portMAX_DELAY))
    {
        while (true)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);  // Atraso de 10 segundos

            sendSensorDataToDashboard();
        }
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // GPIOs for LEDs
    gpio_reset_pin(LED_GREEN_GPIO);
    gpio_set_direction(LED_GREEN_GPIO, GPIO_MODE_OUTPUT);
    
    gpio_reset_pin(LED_RED_GPIO);
    gpio_set_direction(LED_RED_GPIO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_BLUE_GPIO);
    gpio_set_direction(LED_BLUE_GPIO, GPIO_MODE_OUTPUT);

    wifiConnectionSemaphore = xSemaphoreCreateBinary();
    mqttConnectionSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    // Initialize DHT11 sensor
    ESP_LOGI("DHT11", "Initializing DHT11 on GPIO %d", DHT11_PIN);
    DHT11_init(DHT11_PIN);

    // Initialize OLED display
    ESP_LOGI("OLED", "Initializing OLED display on SDA (GPIO 4) and SCL (GPIO 15)");
    oled_display_init(OLED_SDA_PIN, OLED_SCL_PIN);

    xTaskCreate(&wifiConnected, "Wi-Fi Connection", 4096, NULL, 2, NULL);  
    xTaskCreate(&handleServerCommunication, "Server Communication", 4096, NULL, 1, NULL);
}
