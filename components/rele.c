#include "gpio_setup.h"

static gpio_num_t rele_gpio;

void rele_init(gpio_num_t gpio_num)
{
    rele_gpio = gpio_num;
    pinMode(rele_gpio, GPIO_OUTPUT);
}

void rele_on(void)
{
    digitalWrite(rele_gpio, 1);
}

void rele_off(void)
{
    digitalWrite(rele_gpio, 0);
}
