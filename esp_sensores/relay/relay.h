#ifndef RELAY_H
#define RELAY_H
#include "cJSON.h"
#include <stdbool.h>

#define RELAY_GPIO 25  


void init_relay(void);
void set_relay_state(bool state);
void process_relay_command(cJSON *request);


#endif // RELAY_H
