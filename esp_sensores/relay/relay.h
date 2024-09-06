#ifndef RELAY_H
#define RELAY_H
#include "cJSON.h"
#include <stdbool.h>

#define RELAY_GPIO 25  // Definir o GPIO para o controle do relé

// Função para inicializar o relé
void init_relay(void);

// Função para definir o estado do relé (ligado/desligado)
void set_relay_state(bool state);
void process_relay_command(cJSON *request);


#endif // RELAY_H
