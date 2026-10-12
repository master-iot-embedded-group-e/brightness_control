#include <zephyr/kernel.h>
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY 6 // Set higher priority for fast processing

void fsm_thread(void *p1, void *p2, void *p3)
{
    struct ldr_t ldr;
    struct rgb_pwm_t rgb_pwm;

    printk("    FSM thread init   \n");

    while (1) {
        /* K_FOREVER makes it blocking */
        if (k_msgq_get(&q_ldr_data, &ldr, K_FOREVER) == 0) {

            // ldr.reading es un porcentaje. rehacer en otra rama.
            if (ldr.reading > 49) {
                rgb_pwm.blue = 100;
                rgb_pwm.red = 100;
                rgb_pwm.green = 100;
            } else {
                rgb_pwm.blue = 0;
                rgb_pwm.red = 0;
                rgb_pwm.green = 100;
            }

            k_msgq_put(&q_rgb_data, &rgb_pwm, K_MSEC(10));
        }
    }
}

K_THREAD_DEFINE(fsm_thread_id, STACK_SIZE, fsm_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
