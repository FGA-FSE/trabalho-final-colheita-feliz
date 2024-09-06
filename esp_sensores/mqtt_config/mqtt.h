#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>

void mqtt_start_thingsboard();  
void mqtt_start_mosquitto();    
void mqtt_send_message_thingsboard(const char *topic, const char *message);  
void mqtt_send_message_mosquitto(const char *topic, const char *message);   
void set_led_brightness(int green_brightness);

#endif 
