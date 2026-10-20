#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "interface.h"
#include "control.h"
#include "rgb.h"

LOG_MODULE_REGISTER(control, LOG_LEVEL_INF);

K_EVENT_DEFINE(e_enable_devices);

typedef enum
{
	BLUE,
	NORMAL,
	OFF
} Fsm_t;
static const char *state_string[3] = {"BLUE", "NORMAL", "OFF"};

// Colour last written to the LED. Local to this file now that it no longer
// travels between threads.
typedef struct
{
	bool blue;
	bool red;
	bool green;
} rgb_t;

static bool rgb_equal(const rgb_t *a, const rgb_t *b)
{
	return a->red == b->red && a->green == b->green && a->blue == b->blue;
}

static void set_rgb_color(rgb_t *last_rgb, bool red, bool green, bool blue)
{
	const rgb_t rgb = {.red = red, .green = green, .blue = blue};

	// fsm() runs on every LDR sample (2 s): only touch the GPIOs when the
	// colour actually changes.
	if (rgb_equal(&rgb, last_rgb))
	{
		return;
	}

	int err = rgb_set(red, green, blue);
	if (err < 0)
	{
		// last_rgb is kept, so the next fsm() call retries the write.
		LOG_ERR("could not set the RGB LED: %d", err);
		return;
	}
	*last_rgb = rgb;
}

static void fsm(Fsm_t *state, ldr_t *ldr, rgb_t *last_rgb, button_type_of_event_t *button_event)
{
	// State evaluation
	Fsm_t new_state = *state;
	switch (*button_event)
	{
	case BUTTON_SHORT_PRESS:
		new_state = (*state + NORMAL) & 0x01;
		break;

	case BUTTON_LONG_PRESS:
		new_state = OFF;
		break;

	case NO_PRESSED:
		break;

	default:
		//*state = NORMAL;
		LOG_ERR("FSM: Unexpected press button duration: %d", *button_event);
		break;
	}

	// Evaluate and store new state
	if (*state != new_state)
	{
		LOG_INF("fsm: %s", state_string[new_state]);
		if (new_state == OFF)
		{
			LOG_INF("System OFF");
		}
	}
	*state = new_state;

	// Reset event driven variables for the next one
	*button_event = NO_PRESSED;

	// Evaluate outputs
	switch ((uint8_t)*state)
	{
	case NORMAL:
		k_event_post(&e_enable_devices, EV_ADC_ON);
		if (ldr->brightness < 33)
		{
			set_rgb_color(last_rgb, true, false, false);
		}
		else if (ldr->brightness <= 66 && ldr->brightness >= 33)
		{
			set_rgb_color(last_rgb, true, true, false);
		}
		else if (ldr->brightness > 66)
		{
			set_rgb_color(last_rgb, false, true, false);
		}
		break;

	case BLUE:
		set_rgb_color(last_rgb, false, false, true);
		k_event_clear(&e_enable_devices, EV_ADC_ON);
		break;

	case OFF:
		set_rgb_color(last_rgb, false, false, false);
		k_event_clear(&e_enable_devices, EV_ADC_ON);
		break;

	default:
		LOG_WRN("Unexpected state: %d", (uint8_t)*state);
		break;
	}
}

int control_run(void)
{
	ldr_t ldr_data = {
		.brightness = 0,
	};
	Fsm_t fsm_state = NORMAL;
	// The LEDs are configured GPIO_OUTPUT_INACTIVE, so they start off.
	rgb_t last_rgb = {0};
	button_type_of_event_t button_event = NO_PRESSED;

	// Create poll structures
	struct k_poll_event events[2] = {
		K_POLL_EVENT_INITIALIZER(
			K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
			K_POLL_MODE_NOTIFY_ONLY,
			&q_button_data),
		K_POLL_EVENT_INITIALIZER(
			K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
			K_POLL_MODE_NOTIFY_ONLY,
			&q_ldr_data)};

	int err = rgb_init();
	if (err < 0)
	{
		LOG_ERR("could not initialize the RGB LED: %d", err);
		return err;
	}

	// Enable at init the ADC measurements.
	k_event_post(&e_enable_devices, EV_ADC_ON);

	// Wait for the first valid LDR measurement before starting the control logic
	if (k_msgq_get(&q_ldr_data, &ldr_data, K_FOREVER) == 0)
	{
		// Zephyr API requests to reset the event state after attending it.
		events[1].state = K_POLL_STATE_NOT_READY;
	}

	LOG_INF("control init");
	LOG_INF("fsm: %s", state_string[fsm_state]);
	// This first call has ldr data so the LED turns on without additional 2 sec wait
	fsm(&fsm_state, &ldr_data, &last_rgb, &button_event);

	while (1)
	{

		// This is where the thread actually stops until new events
		if (k_poll(events, ARRAY_SIZE(events), K_FOREVER) != 0)
		{
			continue;
		}

		// Zephyr API doc/kernel/services/polling.rst
		if (events[0].state == K_POLL_STATE_MSGQ_DATA_AVAILABLE)
		{
			// We need a loop in case there are several events in he queue
			while (k_msgq_get(&q_button_data, &button_event, K_NO_WAIT) == 0)
			{
				switch (button_event)
				{
				case BUTTON_SHORT_PRESS:
					LOG_INF("short press");
					break;
				case BUTTON_LONG_PRESS:
					LOG_INF("long press");
					break;
				case NO_PRESSED:
					break;
				default:
					LOG_WRN("unexpected button event type: %d", button_event);
					break;
				}
				fsm(&fsm_state, &ldr_data, &last_rgb, &button_event);
				// Zephyr API requests to reset the event state after attending it.
				// doc/kernel/services/polling.rst
				events[0].state = K_POLL_STATE_NOT_READY;
			}
		}
		else if (events[1].state == K_POLL_STATE_MSGQ_DATA_AVAILABLE)
		{
			// Queued lectures would be read in the next lopp as k_poll would return another one.
			if (k_msgq_get(&q_ldr_data, &ldr_data, K_NO_WAIT) == 0)
			{
				fsm(&fsm_state, &ldr_data, &last_rgb, &button_event);
				// Zephyr API requests to reset the event state after attending it.
				events[1].state = K_POLL_STATE_NOT_READY;
			}
		}
	}
}
