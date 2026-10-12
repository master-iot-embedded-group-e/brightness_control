#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <zephyr/drivers/gpio.h>

int gpio_output_init(const struct gpio_dt_spec *gpio);
int gpio_input_init(
    const struct gpio_dt_spec *gpio, 
    struct gpio_callback *callback, 
    gpio_callback_handler_t handler
);
int gpio_power_on(const struct gpio_dt_spec *gpio);
int gpio_power_off(const struct gpio_dt_spec *gpio);
int gpio_get_value(const struct gpio_dt_spec *gpio);

#endif /* GPIO_H */
