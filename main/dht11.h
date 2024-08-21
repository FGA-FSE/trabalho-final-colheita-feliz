#ifndef DHT11_H
#define DHT11_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define DHT_PIN GPIO_NUM_4

bool readDHT11(int *temperature, int *humidity);
void dht_task(void *pvParameter);

#endif
