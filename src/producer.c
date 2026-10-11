#include <zephyr/kernel.h>
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY 7

void producer_thread(void *p1, void *p2, void *p3)
{
    struct ldr_t ldr;

    printk("    Producer thread init   \n");

    while (1) {
        ldr.value_lux = 999.0; // Ejemplo: simulación de lux
        ldr.timestamp = k_uptime_get_32();

        /* Enviamos el dato a la cola. Si está llena, espera hasta 10ms */
        k_msgq_put(&q_ldr_data, &ldr, K_MSEC(10));

        k_msleep(20);
    }
}

/* Macro de Zephyr para registrar y arrancar el hilo automáticamente */
K_THREAD_DEFINE(producer_thread_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
