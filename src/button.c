#include "button.h"
#include "gpio.h"
#include "interface.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(button, LOG_LEVEL_INF);

#define DEBOUNCE_DELAY_MS 60
#define LONG_PRESS_DELAY_MS 1000

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static struct gpio_callback callback;

// Event handling
static uint32_t pressed_at;
static bool held, long_sent;
static void debounce_handler(struct k_work *work);
static void long_handler(struct k_work *work);

// Sync mechanism
K_WORK_DELAYABLE_DEFINE(debounce_work, debounce_handler);
K_WORK_DELAYABLE_DEFINE(long_work, long_handler);

/**
 * Send a button event to the message queue.
 */
static void send_event(button_type_of_event_t event) {
    LOG_DBG("sending event %d", event);
    // Rate-limited: if the consumer stalls, every press would log otherwise.
    if (k_msgq_put(&q_button_data, &event, K_NO_WAIT) != 0)
        LOG_WRN("event queue full, event %d dropped", event);
}

/**
 * K_WORK_DELAYABLE handler for long press events.
 * This function is called when the long press delay 
 * has elapsed to check if the button is still held.
 */
static void long_handler(struct k_work *work) {
    if (held && !long_sent && gpio_get_value(&button) == 1) {
        long_sent = true;
        send_event(BUTTON_LONG_PRESS);
    }
}

/**
 * K_WORK_DELAYABLE handler for debounce events.
 * This function is called after a short delay to handle
 * button state changes and determine if a short or long
 * press event should be sent.
 */
static void debounce_handler(struct k_work *work) {
    int level = gpio_get_value(&button);
    if (level < 0) {
        LOG_ERR("reading button level: %d", level);
        return;
    }
    // Ignore unchanged state (possible with button bouncing).
    if ((level == 1) == held) return;

    held = (level == 1);
    // If the button is now held, start the long press timer.
    if (held) {
        pressed_at = k_uptime_get_32();
        long_sent = false;
        k_work_reschedule(&long_work, K_MSEC(LONG_PRESS_DELAY_MS));
        return;
    }

    // Cancel the long press work since the button is no longer held.
    k_work_cancel_delayable(&long_work);

    // If the button was held long enough to trigger a long press, ignore the short press.
    if (long_sent) return;
    // Calculate the duration the button was held.
    uint32_t duration = k_uptime_get_32() - pressed_at;
    // Determine if the button press qualifies as a long or short press.
    duration >= LONG_PRESS_DELAY_MS 
        ? send_event(BUTTON_LONG_PRESS) 
        : send_event(BUTTON_SHORT_PRESS);
}

static void button_changed(
    const struct device *dev,
    struct gpio_callback *cb,
    gpio_port_pins_t pins
) {
    /**
     * Reschedule the debounce work to handle button state 
     * changes after a short delay. Prevents button bouncing.
     */
    k_work_reschedule(&debounce_work, K_MSEC(DEBOUNCE_DELAY_MS));
}

int button_init(void) {
    return gpio_input_init(&button, &callback, button_changed);
}