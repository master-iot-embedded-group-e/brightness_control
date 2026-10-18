#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "interface.h"
#include "rgb.h"

#define STACK_SIZE 1024
#define PRIORITY 7

LOG_MODULE_REGISTER(actuator, LOG_LEVEL_INF);

void actuator_thread_run(void *p1, void *p2, void *p3)
{
    rgb_t rgb;
    int ret = rgb_init();
    if (ret < 0)
    {
        LOG_ERR("initializing RGB");
        return;
    }

    LOG_INF("thread init");

    while (1)
    {
        if (k_msgq_get(&q_rgb_data, &rgb, K_FOREVER) == 0)
        {
            if (rgb.red)
            {
                red_on();
            }
            else
            {
                red_off();
            }
            if (rgb.green)
            {
                green_on();
            }
            else
            {
                green_off();
            }
            if (rgb.blue)
            {
                blue_on();
            }
            else
            {
                blue_off();
            }
        }
    }
}

K_THREAD_DEFINE(actuator_thread_id, STACK_SIZE, actuator_thread_run,
                NULL, NULL, NULL, PRIORITY, 0, 0);
