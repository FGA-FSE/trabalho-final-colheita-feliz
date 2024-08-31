#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_11
#define SENSOR_PIN ADC_CHANNEL_0

const static char *TAG = "EXAMPLE";

static int adc_raw[2][10];

const int AirValue = 3200;   // Valor de referência para o ar (seco)
const int WaterValue = 1199; // Valor de referência para a água (úmido)
const int SensorPin = SENSOR_PIN;  // GPIO15 no ESP32

void soil_moisture_task(void *pvParameter) {

    //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = EXAMPLE_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, SensorPin, &config));

    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, SensorPin, &adc_raw[0][0]));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, SensorPin, adc_raw[0][0]);

        // Calcular a porcentagem de umidade
        int moisture_percent = (AirValue - adc_raw[0][0]) * 100 / (AirValue - WaterValue);
        if (moisture_percent > 100) {
            moisture_percent = 100;
        } else if (moisture_percent < 0) {
            moisture_percent = 0;
        }
        ESP_LOGI(TAG, "Soil Moisture Percent: %d%%", moisture_percent);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}
