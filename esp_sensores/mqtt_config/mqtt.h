#ifndef MQTT_H
#define MQTT_H

void mqtt_start();
void mqtt_send_message(char *topic, char *message);
void set_led_brightness(int brightness);  // Função para ajustar o brilho do LED

#endif // MQTT_H
