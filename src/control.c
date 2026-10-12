#include <zephyr/kernel.h>
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY 6 // Set higher priority for fast processing

enum{
    NORMAL,
    BLUE,
    OFF
} Fsm_t;

void fsm_thread(void *p1, void *p2, void *p3)
{
    ldr_t ldr;
    rgb_t rgb;
    rgb_t last_rgb = {0};
    bool last_rgb_valid = false;

    printk("    FSM thread init   \n");

    while (1) {
        /* K_FOREVER makes it blocking */
        if (k_msgq_get(&q_ldr_data, &ldr, K_FOREVER) == 0) {
            if (ldr.brightness < 33) {
                rgb.blue = false;
                rgb.red = true;
                rgb.green = false;
            } else if (ldr.brightness <= 66 && ldr.brightness >= 33) {
                rgb.blue = false;
                rgb.red = true;
                rgb.green = true;
            } else if (ldr.brightness > 66) {
                rgb.blue = false;
                rgb.red = false;
                rgb.green = true;
            }

            if (!last_rgb_valid ||
                rgb.red != last_rgb.red ||
                rgb.green != last_rgb.green ||
                rgb.blue != last_rgb.blue) {
                if (k_msgq_put(&q_rgb_data, &rgb, K_MSEC(10)) == 0) {
                    last_rgb = rgb;
                    last_rgb_valid = true;
                }
            }
        }
    }
}

K_THREAD_DEFINE(fsm_thread_id, STACK_SIZE, fsm_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
