#ifndef _RELE
#define _RELE

#include "gpio_setup.h"

void rele_init(gpio_num_t gpio_num);

void rele_on(void);

void rele_off(void);
#endif