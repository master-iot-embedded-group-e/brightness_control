#include <zephyr/kernel.h>
#include "interface.h"
#include "ldr.h"
#include "button.h"

#define STACK_SIZE 1024
#define PRIORITY 7

void producer_thread(void *p1, void *p2, void *p3)
{
    ldr_t ldr;
	int err = 0;

    printk("    Producer thread init   \n");

	err = ldr_init();
    if (err < 0) {
        printk("ERROR: could not initialize LDR. %d\n", err);
		printk("ERROR: Producer will not start\n");
        return;
    }

	err = button_init();
	if (err < 0) {
		printk("ERROR: could not initialize the buttons. %d\n", err);
		printk("ERROR: Producer will not start\n");
		return;
    }

    while (1) {
        uint8_t ldr_percentage;
        int ret = ldr_read(&ldr_percentage);
        if (ret < 0) {
            printk("ERROR: could not read LDR: %d\n", ret);
            return;
        }
        printk("LDR reading as percentage: %d%%\n", ldr_percentage);
        // Convert LDR reading to brightness (inverted percentage)
        ldr.brightness = 100 - ldr_percentage; 
        ldr.timestamp = k_uptime_get_32();

        // Send the LDR data to the message queue. If the queue is full, wait up to 10ms.
        k_msgq_put(&q_ldr_data, &ldr, K_MSEC(10));

        k_msleep(2000);
    }
}

// Register and start the producer thread automatically using Zephyr's macro.
K_THREAD_DEFINE(producer_thread_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
