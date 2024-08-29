#ifndef MQTT_APP_H
#define MQTT_APP_H

#include "mqtt_client.h"


void mqtt_app_start(void);

extern esp_mqtt_client_handle_t client;

#endif
