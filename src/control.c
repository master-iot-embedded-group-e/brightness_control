#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY   6 // Set higher priority for fast processing

LOG_MODULE_REGISTER(control, LOG_LEVEL_INF);

typedef enum {
	BLUE,
	NORMAL,
	OFF
} Fsm_t;

typedef struct {
	rgb_t rgb;
	rgb_t last_rgb;
	bool new_rgb_values;
} Rgb_state_t;

static bool rgb_equal(const rgb_t *a, const rgb_t *b)
{
	return a->red == b->red && a->green == b->green && a->blue == b->blue;
}

static void set_rgb_color(Rgb_state_t *rgb_state, bool red, bool green, bool blue)
{
	rgb_state->rgb.blue = blue;
	rgb_state->rgb.red = red;
	rgb_state->rgb.green = green;
	rgb_state->new_rgb_values = !rgb_equal(&rgb_state->rgb, &rgb_state->last_rgb);
}

static void fsm(Fsm_t *state, ldr_t *ldr, Rgb_state_t *rgb_state, button_type_of_event_t *button_event)
{
	switch (*button_event)
	{
	case BUTTON_SHORT_PRESS:
		*state = (*state + NORMAL) & 0x01;
		break;

	case BUTTON_LONG_PRESS:
		*state = OFF;
		break;

	case NO_PRESSED:
		break;
	
	default:
		//*state = NORMAL;
		LOG_ERR("FSM: Unexpected press button duration: %d", *button_event);
		break;
	}

	// Reset event driven variables for the next one
	*button_event = NO_PRESSED;

	switch ((uint8_t)*state) {
	case NORMAL:
		LOG_DBG("fsm: NORMAL");		
		if (ldr->brightness < 33) {
			set_rgb_color(rgb_state, true, false, false);

		} else if (ldr->brightness <= 66 && ldr->brightness >= 33) {
			set_rgb_color(rgb_state, true, true, false);

		} else if (ldr->brightness > 66) {
			set_rgb_color(rgb_state, false, true, false);
		}
		break;

	case BLUE:
		LOG_DBG("fsm: BLUE");		
		set_rgb_color(rgb_state, false, true, true);
		break;

	case OFF:
		LOG_DBG("fsm: OFF");		
		set_rgb_color(rgb_state, false, false, false);
		break;

	default:
		LOG_ERR("Unexpected state: %d", (uint8_t)*state);
		break;
	}

	if (rgb_state->new_rgb_values == false) {
		// return to avoid waken up the actuator thread
		return;
	}

	if (k_msgq_put(&q_rgb_data, &rgb_state->rgb, K_MSEC(10)) == 0) {
		rgb_state->last_rgb = rgb_state->rgb;
	}
}

void control_thread(void *p1, void *p2, void *p3)
{
	ldr_t ldr_data = {
		.brightness = 0,
	};
	Fsm_t fsm_state = NORMAL;
	Rgb_state_t rgb_state = {
		.last_rgb = {0},
		.rgb = {0},
		.new_rgb_values = false,
	};
    button_type_of_event_t button_event = NO_PRESSED;
	
	// Create poll structures
    struct k_poll_event events[2] = {
		K_POLL_EVENT_INITIALIZER(
			K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
            K_POLL_MODE_NOTIFY_ONLY, 
            &q_button_data
        ),
        K_POLL_EVENT_INITIALIZER(
			K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
            K_POLL_MODE_NOTIFY_ONLY, 
            &q_ldr_data
        )
    };

	// Wait for the first valid LDR measurement before starting the control logic
	if (k_msgq_get(&q_ldr_data, &ldr_data, K_FOREVER) == 0) { 
		// Zephyr API requests to reset the event state after attending it.
		events[1].state = K_POLL_STATE_NOT_READY;
	}
	
	LOG_INF("thread init");

	while (1) {

		// This is where the thread actually stops until new events
		if (k_poll(events, ARRAY_SIZE(events), K_FOREVER) != 0) {
			continue;
		}

		// Zephyr API doc/kernel/services/polling.rst
		if(events[0].state == K_POLL_STATE_MSGQ_DATA_AVAILABLE)
		{
			// We need a loop in case there are several events in he queue
			while (k_msgq_get(&q_button_data, &button_event, K_NO_WAIT) == 0) {
				switch (button_event) {
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
				fsm(&fsm_state, &ldr_data, &rgb_state, &button_event);
				// Zephyr API requests to reset the event state after attending it.
				// doc/kernel/services/polling.rst
				events[0].state = K_POLL_STATE_NOT_READY;
			}
		}
		else if(events[1].state == K_POLL_STATE_MSGQ_DATA_AVAILABLE)
		{
			// Queued lectures would be read in the next lopp as k_poll would return another one.
			if (k_msgq_get(&q_ldr_data, &ldr_data, K_NO_WAIT) == 0) { 
				fsm(&fsm_state, &ldr_data, &rgb_state, &button_event);
				// Zephyr API requests to reset the event state after attending it.
				events[1].state = K_POLL_STATE_NOT_READY;
			}
		}				
	}
}

K_THREAD_DEFINE(control_thread_id, STACK_SIZE, control_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
