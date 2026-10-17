#include <zephyr/kernel.h>
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY   6 // Set higher priority for fast processing

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

static void fsm(Fsm_t *state, ldr_t *ldr, Rgb_state_t *rgb_state, button_type_of_event_t button_event)
{
	switch ((button_event))
	{
	case BUTTON_SHORT_PRESS:
		*state = (*state + NORMAL) & 0x01;
		break;

	case BUTTON_LONG_PRESS:
		*state = OFF;
		break;
	
	default:
		*state = NORMAL;
		printk("FSM: Unexpected press button duration\n");
		break;
	}

	switch ((uint8_t)*state) {
	case NORMAL:
		if (ldr->brightness < 33) {
			set_rgb_color(rgb_state, true, false, false);

		} else if (ldr->brightness <= 66 && ldr->brightness >= 33) {
			set_rgb_color(rgb_state, true, true, false);

		} else if (ldr->brightness > 66) {
			set_rgb_color(rgb_state, false, true, false);
		}
		break;

	case BLUE:
		set_rgb_color(rgb_state, false, true, true);
		break;

	case OFF:
		set_rgb_color(rgb_state, false, false, false);
		break;

	default:
		printk("FSM: Unexpected state\n");
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

void fsm_thread(void *p1, void *p2, void *p3)
{
	ldr_t ldr_data;
	Fsm_t fsm_state = NORMAL;
	Rgb_state_t rgb_state = {
		.last_rgb = {0},
		.rgb = {0},
		.new_rgb_values = false,
	};

	printk("    FSM thread init   \n");

	button_init();
    button_type_of_event_t button_event;
    struct k_poll_event events[] = {
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

	while (1) {

		if (k_poll(events, ARRAY_SIZE(events), K_FOREVER) != 0) {
			continue;
		}

		while (k_msgq_get(&q_button_data, &button_event, K_NO_WAIT) == 0) {
			switch (button_event) {
			case BUTTON_SHORT_PRESS:
				printk("FSM: short press\n");
				break;
			case BUTTON_LONG_PRESS:
				printk("FSM: long press\n");
				break;
			default:
				printk("FSM: Unexpected button event type\n");
			break;
			}			
		}

		/* K_FOREVER makes it blocking */
		if (k_msgq_get(&q_ldr_data, &ldr_data, K_NO_WAIT) == 0) {
			fsm(&fsm_state, &ldr_data, &rgb_state, button_event);
		}
	}
}

K_THREAD_DEFINE(fsm_thread_id, STACK_SIZE, fsm_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
