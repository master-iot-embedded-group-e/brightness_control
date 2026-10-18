// Primero el propio header: así el compilador comprueba que las definiciones
// de este fichero coinciden con las declaraciones públicas.
#include "gpio.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(gpio, LOG_LEVEL_INF);

static int gpio_init(const struct gpio_dt_spec *gpio, gpio_flags_t flags)
{
    if (!gpio_is_ready_dt(gpio))
    {
        LOG_ERR("%s pin %d: port not ready", gpio->port->name, gpio->pin);
        return -ENODEV;
    }

    int ret;
    ret = gpio_pin_configure_dt(gpio, flags);
    if (ret < 0)
    {
        LOG_ERR("%s pin %d: configure failed: %d", gpio->port->name, gpio->pin, ret);
        return ret;
    }

    return 0;
}

int gpio_output_init(const struct gpio_dt_spec *gpio)
{
    return gpio_init(gpio, GPIO_OUTPUT_INACTIVE);
}

int gpio_input_init(const struct gpio_dt_spec *gpio, struct gpio_callback *callback, gpio_callback_handler_t handler)
{
    int ret;
    ret = gpio_init(gpio, GPIO_INPUT);
    if (ret < 0)
    {
        LOG_ERR("%s pin %d: input init failed: %d", gpio->port->name, gpio->pin, ret);
        return ret;
    }

    gpio_init_callback(callback, handler, BIT(gpio->pin));

    ret = gpio_add_callback(gpio->port, callback);
    if (ret < 0)
    {
        LOG_ERR("%s pin %d: adding callback failed: %d", gpio->port->name, gpio->pin, ret);
        return ret;
    }
    ret = gpio_pin_interrupt_configure_dt(gpio, GPIO_INT_EDGE_BOTH);
    if (ret < 0)
    {
        LOG_ERR("%s pin %d: configuring interrupt failed: %d", gpio->port->name, gpio->pin, ret);
        return ret;
    }

    return ret;
}

int gpio_power_on(const struct gpio_dt_spec *gpio)
{
    int ret;
    ret = gpio_pin_set_dt(gpio, 1);
    if (ret < 0)
    {
        LOG_ERR("%s pin %d: set active failed: %d", gpio->port->name, gpio->pin, ret);
        return ret;
    }

    return 0;
}

int gpio_power_off(const struct gpio_dt_spec *gpio)
{
    int ret;
    ret = gpio_pin_set_dt(gpio, 0);
    if (ret < 0)
    {
        LOG_ERR("%s pin %d: set inactive failed: %d", gpio->port->name, gpio->pin, ret);
        return ret;
    }

    return 0;
}

int gpio_get_value(const struct gpio_dt_spec *gpio)
{
    return gpio_pin_get_dt(gpio);
}
