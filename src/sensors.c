#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "interface.h"
#include "ldr.h"
#include "button.h"

#define STACK_SIZE 1024
#define PRIORITY 7
#define LDR_PERIOD_MS 2000

LOG_MODULE_REGISTER(sensors, LOG_LEVEL_INF);

// No expiry function: the thread synchronizes with k_timer_status_sync().
K_TIMER_DEFINE(ldr_timer, NULL, NULL);

void producer_thread(void *p1, void *p2, void *p3)
{
	ldr_t ldr;
	uint8_t ldr_percentage;
	int err = 0;

	err = ldr_init();
	if (err < 0)
	{
		LOG_ERR("ERROR: could not initialize LDR. %d", err);
		LOG_ERR("ERROR: Producer will not start");
		return;
	}

	err = button_init();
	if (err < 0)
	{
		LOG_ERR("ERROR: could not initialize the buttons. %d", err);
		LOG_ERR("ERROR: Producer will not start");
		return;
	}

	LOG_INF("thread init");

	// Periodic timer instead of k_msleep(): k_msleep() counts from the moment it
	// is called, so the time spent reading the ADC and logging is added to every
	// period and accumulates as drift. A periodic k_timer re-arms itself inside
	// its expiry handler with an absolute timeout (kernel/timer.c, needs
	// CONFIG_TIMEOUT_64BIT), so expirations stay on exact multiples of the period.
	// K_NO_WAIT: first reading right away, so the LED turns on without a 2 s wait.
	k_timer_start(&ldr_timer, K_NO_WAIT, K_MSEC(LDR_PERIOD_MS));

	while (1)
	{
		// The ADC is read here, in thread context, and not in a timer expiry
		// function: that runs in ISR context, where adc_read() cannot block.
		k_timer_status_sync(&ldr_timer);

		err = ldr_read(&ldr_percentage);
		if (err < 0)
		{
			LOG_ERR("ERROR: could not read LDR: %d", err);
			return;
		}
		// Convert LDR reading to brightness (inverted percentage)
		ldr.brightness = 100 - ldr_percentage;
		LOG_INF("LDR reading: %d%%", ldr.brightness);

		// Send the LDR data to the message queue. If the queue is full, wait up to 10ms.
		k_msgq_put(&q_ldr_data, &ldr, K_MSEC(10));

		// If the ADC is marked as disabled, stop the timer and wait here. A running
		// timer would wake the CPU from Stop mode every period for nothing, and its
		// expiries would pile up in its status count while nobody reads.
		if (!k_event_test(&e_enable_devices, EV_ADC_ON))
		{
			k_timer_stop(&ldr_timer);
			k_event_wait(&e_enable_devices, EV_ADC_ON, false, K_FOREVER);
			// Read right away when sampling is enabled again.
			k_timer_start(&ldr_timer, K_NO_WAIT, K_MSEC(LDR_PERIOD_MS));
		}
	}
}

// Register and start the producer thread automatically using Zephyr's macro.
K_THREAD_DEFINE(producer_thread_id, STACK_SIZE, producer_thread,
				NULL, NULL, NULL, PRIORITY, 0, 0);
