// Primero el propio header: así el compilador comprueba que las definiciones
// de este fichero coinciden con las declaraciones públicas.
#include "gpio.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/printk.h>
#include <errno.h>
#include <zephyr/drivers/gpio.h>

int gpio_init(const struct gpio_dt_spec *gpio) {
    if (!gpio_is_ready_dt(gpio)) {
        printk("ERROR: GPIO not ready\n");
        return -1;
    }

    int ret;
    ret = gpio_pin_configure_dt(gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR GPIO: %d\n", ret);
        return -ENODEV;
    }

    return 0;
}

int gpio_power_on (const struct gpio_dt_spec *gpio) {
    int ret;
    ret = gpio_pin_set_dt(gpio, 1);
    if (ret < 0) {
        printk("ERROR GPIO power on: %d\n", ret);
        return -ENODEV;
    }

    return 0;
}

int gpio_power_off (const struct gpio_dt_spec *gpio) {
    int ret;
    ret = gpio_pin_set_dt(gpio, 0);
    if (ret < 0) {
        printk("ERROR GPIO power off: %d\n", ret);
        return -ENODEV;
    }

    return 0;
}