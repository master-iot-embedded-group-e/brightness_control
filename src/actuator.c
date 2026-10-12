#include <zephyr/kernel.h>
#include "interface.h"
#include "rgb.h"

#define STACK_SIZE 1024
#define PRIORITY 7

void hilo_actuador_run(void *p1, void *p2, void *p3)
{
    struct rgb_pwm_t rgb_pwm;
    int ret = rgb_init();
    if (ret < 0) {
        printk("Error initializing RGB\n");
        return;
    }

    printk("    Actuator thread init   \n");

    while (1) {
        if (k_msgq_get(&q_rgb_data, &rgb_pwm, K_FOREVER) == 0) {
            red_on();
            green_on();
            blue_on();
        }
    }
}

K_THREAD_DEFINE(hilo_actuador_id, STACK_SIZE, hilo_actuador_run,
                NULL, NULL, NULL, PRIORITY, 0, 0);
