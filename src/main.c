#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "interface.h"

// Instantiating message queues for LDR, RGB, and Button data
K_MSGQ_DEFINE(q_ldr_data, sizeof(struct ldr_t), MAX_NUMBER_OF_MESSAGES, 1);
K_MSGQ_DEFINE(q_rgb_data, sizeof(struct rgb_t), MAX_NUMBER_OF_MESSAGES, 1);
K_MSGQ_DEFINE(q_button_data, sizeof(struct button_events_t), MAX_NUMBER_OF_MESSAGES, 1);

int main(void)
{
    printk("Embedded Platforms and Communications for IoT\n");
    printk("Brightness Control\n");
    // Keep the main thread idle after initialization
    while (1) {
        k_sleep(K_FOREVER);
    }
    return 0;
}
