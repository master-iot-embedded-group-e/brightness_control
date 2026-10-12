#include "button.h"
#include "gpio.h"
#include "interface.h"
#include <zephyr/sys/atomic.h>
#include <zephyr/kernel.h>

#define DEBOUNCE_DELAY_MS 30
#define LONG_PRESS_DELAY_MS 1000

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static struct gpio_callback callback;
static atomic_t edge_at;
static uint32_t pressed_at;
static bool held, long_sent;
static void debounce_handler(struct k_work *work);
static void long_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(debounce_work, debounce_handler);
K_WORK_DELAYABLE_DEFINE(long_work, long_handler);

static void send_event(enum button_event event) {
    struct button_events_t message = {
        .event = event,
        .timestamp = k_uptime_get_32()};
    if (k_msgq_put(&q_button_data, &message, K_NO_WAIT) != 0)
        printk("Button event queue full\n");
}

static void long_handler(struct k_work *work) {
    if (held && !long_sent && gpio_pin_get_dt(&button) == 1) {
        long_sent = true;
        send_event(BUTTON_LONG_PRESS);
    }
}

static void debounce_handler(struct k_work *work) {
    int level = gpio_get_value(&button);
    uint32_t instant = (uint32_t)atomic_get(&edge_at);
    if (level < 0)
        return;
    if (level == 1) {
        if (!held) {
            held = true;
            long_sent = false;
            pressed_at = instant;
        }
        if (!long_sent) {
            uint32_t elapsed = k_uptime_get_32() - pressed_at;
            k_work_reschedule(
                &long_work,
                K_MSEC(
                    elapsed >= LONG_PRESS_DELAY_MS 
                        ? 0 
                        : LONG_PRESS_DELAY_MS - elapsed
                )
            );
        }
    }
    else if (held) {
        held = false;
        k_work_cancel_delayable(&long_work);
        if (!long_sent)
            send_event(
                instant - pressed_at >= LONG_PRESS_DELAY_MS
                    ? BUTTON_LONG_PRESS
                    : BUTTON_SHORT_PRESS
                );
    }
}

static void button_changed(
    const struct device *dev,
    struct gpio_callback *cb,
    gpio_port_pins_t pins
) {
    atomic_set(&edge_at, (atomic_val_t)k_uptime_get_32());
    k_work_reschedule(&debounce_work, K_MSEC(DEBOUNCE_DELAY_MS));
}

int button_init(void) {
    return gpio_input_init(&button, &callback, button_changed);
}