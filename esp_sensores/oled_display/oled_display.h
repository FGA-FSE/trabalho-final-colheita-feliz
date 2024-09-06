#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H


#define OLED_SDA_PIN 4  
#define OLED_SCL_PIN 15 


void oled_display_init(int sda_pin, int scl_pin);


void oled_display_update(int temperature, int humidity, int soil_moisture);


void oled_display_soil_moisture(int soilmoisturepercent);

#endif 
