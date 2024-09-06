#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>


void mqtt_start();


int mqtt_send_message(const char *topic, const char *message);


bool mqtt_wait_for_message();

#endif // MQTT_H
