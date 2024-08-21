#include "dht11.h"
#include <stdio.h>
#include "esp_log.h"

// Função para ler os dados do DHT11
bool readDHT11(int *temperature, int *humidity)
{
    uint8_t data[5] = {0, 0, 0, 0, 0};
    int lastState = 1;
    int counter = 0;
    int j = 0, i;

    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN, 0);  // Puxar para baixo por 18ms
    vTaskDelay(20 / portTICK_PERIOD_MS);

    gpio_set_level(DHT_PIN, 1);  // Puxar para cima por 40us
    ets_delay_us(40);

    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);

    for (i = 0; i < 85; i++)
    {
        counter = 0;
        while (gpio_get_level(DHT_PIN) == lastState)
        {
            counter++;
            ets_delay_us(1);
            if (counter == 255)
                break;
        }
        lastState = gpio_get_level(DHT_PIN);

        if (counter == 255)
            break;

        if ((i >= 4) && (i % 2 == 0))
        {
            data[j / 8] <<= 1;
            if (counter > 16)
                data[j / 8] |= 1;
            j++;
        }
    }

    if ((j >= 40) &&
        (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
    {
        *humidity = data[0];
        *temperature = data[2];
        return true;
    }
    else
    {
        return false;
    }
}

void dht_task(void *pvParameter)
{
    int temperature = 0;
    int humidity = 0;
    char payload[100];

    while (1)
    {
        if (readDHT11(&temperature, &humidity))
        {
            printf("Temperatura: %d°C, Umidade: %d%%\n", temperature, humidity);
            sprintf(payload, "{\"temperature\": %d, \"humidity\": %d}", temperature, humidity);
            esp_mqtt_client_publish(client, "v1/devices/me/telemetry", payload, 0, 1, 0);
        }
        else
        {
            printf("Erro ao ler o DHT11\n");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
