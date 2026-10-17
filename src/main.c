#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "interface.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// Instantiating message queues for LDR, RGB, and Button data
K_MSGQ_DEFINE(q_ldr_data, sizeof(ldr_t), MAX_NUMBER_OF_MESSAGES, 1);
K_MSGQ_DEFINE(q_rgb_data, sizeof(rgb_t), MAX_NUMBER_OF_MESSAGES, 1);
K_MSGQ_DEFINE(q_button_data, sizeof(button_type_of_event_t), MAX_NUMBER_OF_MESSAGES, 1);

int main(void)
{
    LOG_INF("\n\n \
              Embedded Platforms and Communications for IoT\n \
              Brightness Control\n \
              GitHub Repo: \n \
                https://github.com/master-iot-embedded-group-e/brightness_control\n \
              Authors:\n \
                - Omar Pineiro\n \
                - Carlos Egea\n\n");
    // Keep the main thread idle after initialization
    while (1) {
        k_sleep(K_FOREVER);
        LOG_ERR("main woke up\n");
    }
    return 0;
}
