#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <zephyr/drivers/gpio.h>

int gpio_init(const struct gpio_dt_spec *gpio);
int gpio_power_on(const struct gpio_dt_spec *gpio);
int gpio_power_off(const struct gpio_dt_spec *gpio);

#endif /* GPIO_H */
