#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY 7

void hilo_actuador_run(void *p1, void *p2, void *p3)
{
    struct rgb_pwm_t rgb_pwm;
    static const struct gpio_dt_spec led_blinky = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);


    while (1) {
        if (k_msgq_get(&q_rgb_data, &rgb_pwm, K_FOREVER) == 0) {

            if (rgb_pwm.blue) {
                gpio_pin_set_dt(&led_blinky, 1);
            } else {
                gpio_pin_set_dt(&led_blinky, 0);
            }
        }
    }
}

K_THREAD_DEFINE(hilo_actuador_id, STACK_SIZE, hilo_actuador_run,
                NULL, NULL, NULL, PRIORITY, 0, 0);
