#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include "interface.h"
#include <errno.h>

static const struct pwm_dt_spec red = PWM_DT_SPEC_GET(DT_ALIAS(red_pwm_led));
static const struct pwm_dt_spec green = PWM_DT_SPEC_GET(DT_ALIAS(green_pwm_led));
static const struct pwm_dt_spec blue = PWM_DT_SPEC_GET(DT_ALIAS(blue_pwm_led));

/*
 * uint8_t to pulse in ns
 * as period * 255 can overflow 32 bits we do the division first
 * (period / 255) * level 
 */
static int set_level(const struct pwm_dt_spec *rgb_led, const char *name, uint8_t level)
{
    uint32_t period_steps = rgb_led->period * level;
    uint32_t pulse_ns =  (period_steps / UINT8_MAX);

    int err = pwm_set_pulse_dt(rgb_led, pulse_ns);

    if (err < 0) {
        printk("%s: pwm_set failed (%d)\n", name, err);
        return err;
    }
    return 0;
}

int set_red_level(uint8_t level) {
    return set_level(&red, "red", level);
}

int set_green_level(uint8_t level) {
    return set_level(&green, "green", level);
}

int set_blue_level(uint8_t level) {
    return set_level(&blue, "blue", level);
}

int check_pwm_ready(void) {
    if (!pwm_is_ready_dt(&red) || !pwm_is_ready_dt(&green) || !pwm_is_ready_dt(&blue)) {
        printk("Error: PWM device not ready\n");
        return -ENODEV;
    }
    return 0;
}
