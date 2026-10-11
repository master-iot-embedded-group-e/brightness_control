#include <zephyr/kernel.h>
#include "interface.h"
#include "ldr.h"

#define STACK_SIZE 1024
#define PRIORITY 7

void producer_thread(void *p1, void *p2, void *p3)
{
    struct ldr_t ldr;

    printk("    Producer thread init   \n");
    if (ldr_init() < 0) {
        printk("ERROR: could not initialize LDR\n");
        return;
    }

    while (1) {
        int16_t ret;
        ret = ldr_read();
        if (ret < 0) {
            printk("ERROR: could not read LDR: %d\n", ret);
            return;
        }
        uint16_t ldr_reading = ret;
        printk("LDR reading: %d\n", ldr_reading);
        printk("LDR reading as percentage: %d%%\n", raw_to_percentage(ldr_reading));
        ldr.reading = ldr_reading; // Ejemplo: simulación de lux
        ldr.timestamp = k_uptime_get_32();

        /* Enviamos el dato a la cola. Si está llena, espera hasta 10ms */
        k_msgq_put(&q_ldr_data, &ldr, K_MSEC(10));

        k_msleep(2000);
    }
}

/* Macro de Zephyr para registrar y arrancar el hilo automáticamente */
K_THREAD_DEFINE(producer_thread_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
