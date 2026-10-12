#include <zephyr/kernel.h>
#include "interface.h"
#include "button.h"

#define STACK_SIZE 1024
#define PRIORITY 6 // Set higher priority for fast processing

void fsm_thread(void *p1, void *p2, void *p3)
{
    struct ldr_t ldr;
    struct rgb_t rgb;
    struct rgb_t last_rgb = {0};
    bool last_rgb_valid = false;

    button_init();
    struct button_events_t button_event;
    struct k_poll_event events[] = {
        K_POLL_EVENT_INITIALIZER(
            K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
            K_POLL_MODE_NOTIFY_ONLY, 
            &q_button_data
        ),
        K_POLL_EVENT_INITIALIZER(
            K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
            K_POLL_MODE_NOTIFY_ONLY, 
            &q_ldr_data
        )
    };

    printk("    FSM thread init   \n");

    while (1) {
        events[0].state = K_POLL_STATE_NOT_READY;
        events[1].state = K_POLL_STATE_NOT_READY;

        if (k_poll(events, ARRAY_SIZE(events), K_FOREVER) != 0) {
            continue;
        }

        while (k_msgq_get(&q_button_data, &button_event, K_NO_WAIT) == 0) {
            switch (button_event.event) {
            case BUTTON_SHORT_PRESS:
                printk("FSM: short press\n");
                break;
            case BUTTON_LONG_PRESS:
                printk("FSM: long press\n");
                break;
            }
        }

        if (k_msgq_get(&q_ldr_data, &ldr, K_NO_WAIT) == 0) {
            rgb.blue = false;

            if (ldr.brightness < 33) {
                rgb.red = true;
                rgb.green = false;
            } else if (ldr.brightness <= 66) {
                rgb.red = true;
                rgb.green = true;
            } else {
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
