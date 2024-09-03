#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#include "wifi.h"
#include "mqtt.h"
#include "dht11.h"
#include "adc_module.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "waterSensor.h"


#define DHT11_PIN 5

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

void sendTemperatureAndHumidityToDashboard()
{
  char message[128];
  struct dht11_reading dht11_value = DHT11_read();

  if (dht11_value.status == DHT11_OK)
  {
    sprintf(message, "{\"temperature\": %d, \"humidity\": %d}", dht11_value.temperature, dht11_value.humidity);
    mqtt_send_message("v1/devices/me/telemetry", message);

    ESP_LOGI("DHT11", "Temperature: %d, Humidity: %d", dht11_value.temperature, dht11_value.humidity);
  }
  else
  {
    ESP_LOGE("DHT11", "Failed to read from DHT11 sensor");
  }
}

void handleServerCommunication(void *params)
{
  if (xSemaphoreTake(mqttConnectionSemaphore, portMAX_DELAY))
  {
    while (true)
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      sendTemperatureAndHumidityToDashboard();

      sendWaterLevel();
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

  wifiConnectionSemaphore = xSemaphoreCreateBinary();
  mqttConnectionSemaphore = xSemaphoreCreateBinary();
  wifi_start();

  // Initialize DHT11
  DHT11_init(DHT11_PIN);

  adc_init(ADC_UNIT_1);

  xTaskCreate(&wifiConnected, "Wi-Fi Connection", 4096, NULL, 1, NULL);
  xTaskCreate(&handleServerCommunication, "Server Communication", 4096, NULL, 1, NULL);

}