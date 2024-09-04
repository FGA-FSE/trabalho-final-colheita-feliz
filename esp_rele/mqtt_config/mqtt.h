#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H
#include "esp_event.h" 
void mqtt_start(void);
void mqtt_event_handler_cb(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);

#endif // MQTT_CONFIG_H
