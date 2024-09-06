#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>

// Função para iniciar o cliente MQTT
void mqtt_start();

// Função para enviar uma mensagem via MQTT, retorna o ID da mensagem
int mqtt_send_message(const char *topic, const char *message);

// Função para aguardar o recebimento de uma mensagem via MQTT
bool mqtt_wait_for_message();

#endif // MQTT_H
