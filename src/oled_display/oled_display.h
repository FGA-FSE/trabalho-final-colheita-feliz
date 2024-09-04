#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

// Definições de pinos para o OLED
#define OLED_SDA_PIN 4  // GPIO para SDA (D4)
#define OLED_SCL_PIN 15 // GPIO para SCL (D15)

// Função para inicializar o display OLED com os pinos SDA e SCL especificados
void oled_display_init(int sda_pin, int scl_pin);

// Função para atualizar o display OLED com temperatura, umidade e umidade do solo
void oled_display_update(int temperature, int humidity, int soil_moisture);

// Função para exibir a umidade do solo no display OLED
void oled_display_soil_moisture(int soilmoisturepercent);

#endif // OLED_DISPLAY_H
