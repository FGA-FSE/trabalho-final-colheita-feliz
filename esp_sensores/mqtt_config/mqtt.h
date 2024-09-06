#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>

void mqtt_start_thingsboard();  // Conexão com ThingsBoard
void mqtt_start_mosquitto();    // Conexão com Mosquitto
void mqtt_send_message_thingsboard(const char *topic, const char *message);  // Enviar mensagem para ThingsBoard
void mqtt_send_message_mosquitto(const char *topic, const char *message);    // Enviar mensagem para Mosquitto

// Função para ajustar o brilho do LED verde
void set_led_brightness(int green_brightness);

#endif // MQTT_H
