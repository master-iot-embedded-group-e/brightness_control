#include <zephyr/kernel.h>
#include "interface.h"
#include "ldr.h"

#define STACK_SIZE 1024
#define PRIORITY 7

void producer_thread(void *p1, void *p2, void *p3)
{
    sensor_data_t sensor_data;

    printk("    Producer thread init   \n");
    if (ldr_init() < 0) {
        printk("ERROR: could not initialize LDR\n");
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
        sensor_data.ldr.brightness = 100 - ldr_percentage;
        sensor_data.ldr.timestamp = k_uptime_get_32();

        /* Enviamos el dato a la cola. Si está llena, espera hasta 10ms */
        k_msgq_put(&q_sensor_data, &sensor_data, K_MSEC(10));

        k_msleep(2000);
    }
}

/* Macro de Zephyr para registrar y arrancar el hilo automáticamente */
K_THREAD_DEFINE(producer_thread_id, STACK_SIZE, producer_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
