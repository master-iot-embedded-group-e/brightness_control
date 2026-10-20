// Primero el propio header: así el compilador comprueba que las definiciones
// de este fichero coinciden con las declaraciones públicas.
#include "rgb.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "gpio.h"

static const struct gpio_dt_spec red =
    GPIO_DT_SPEC_GET(DT_ALIAS(red), gpios);
static const struct gpio_dt_spec green =
    GPIO_DT_SPEC_GET(DT_ALIAS(green), gpios);
static const struct gpio_dt_spec blue =
    GPIO_DT_SPEC_GET(DT_ALIAS(blue), gpios);

int rgb_init(void)
{
    int ret;
    ret = gpio_output_init(&red);
    if (ret < 0)
        return ret;

    ret = gpio_output_init(&green);
    if (ret < 0)
        return ret;

    ret = gpio_output_init(&blue);
    if (ret < 0)
        return ret;

    return 0;
}

int red_on(void)
{
    return gpio_power_on(&red);
}

int red_off(void)
{
    return gpio_power_off(&red);
}

int green_on(void)
{
    return gpio_power_on(&green);
}

int green_off(void)
{
    return gpio_power_off(&green);
}

int blue_on(void)
{
    return gpio_power_on(&blue);
}

int blue_off(void)
{
    return gpio_power_off(&blue);
}

int rgb_set(bool red, bool green, bool blue)
{
    // A GPIO write is a register access (microseconds, never blocks), so the
    // caller can drive the LED directly instead of handing it to a thread.
    int ret = red ? red_on() : red_off();
    if (ret < 0)
        return ret;

    ret = green ? green_on() : green_off();
    if (ret < 0)
        return ret;

    return blue ? blue_on() : blue_off();
}
