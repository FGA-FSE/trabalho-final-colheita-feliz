#ifndef WATERSENSOR_H
#define WATERSENSOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void water_sensor_task(void *pvParameter);

#endif