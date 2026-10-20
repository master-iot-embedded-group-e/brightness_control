#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "interface.h"
#include "control.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

// Instantiating message queues for LDR and Button data
K_MSGQ_DEFINE(q_ldr_data, sizeof(ldr_t), MAX_NUMBER_OF_MESSAGES, 1);
K_MSGQ_DEFINE(q_button_data, sizeof(button_type_of_event_t), MAX_NUMBER_OF_MESSAGES, 1);

int main(void)
{
	printk("  Embedded Platforms and Communications for IoT\n \
              Brightness Control\n \
              GitHub Repo: \n \
                https://github.com/master-iot-embedded-group-e/brightness_control\n \
              Authors:\n \
                - Omar Pineiro\n \
                - Carlos Egea\n\n");
	// The main thread runs the control logic instead of sleeping forever: its
	// stack (CONFIG_MAIN_STACK_SIZE) is reused rather than adding another thread.
	// control_run() only returns if initialization failed.
	int err = control_run();
	LOG_ERR("control stopped: %d", err);
	return err;
}
