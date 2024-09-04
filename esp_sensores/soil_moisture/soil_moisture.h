#ifndef SOIL_MOISTURE_H
#define SOIL_MOISTURE_H

// Função para inicializar o sensor de umidade do solo
void init_soil_moisture(int pin);

// Função para ler o valor de umidade do solo
// Retorna a porcentagem de umidade do solo (0% - 100%)
int read_soil_moisture();

#endif // SOIL_MOISTURE_H
