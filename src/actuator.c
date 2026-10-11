#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY 7

/*
 * uint8_t to pulse in ns
 * as period * 255 can overflow 32 bits we do the division first
 * (period / 255) * level 
 */
static void set_level(const struct pwm_dt_spec *rgb_led, const char *name, uint8_t level)
{
    uint32_t period_steps = rgb_led->period * level;
    uint32_t pulse_ns =  (period_steps / UINT8_MAX);

    int err = pwm_set_pulse_dt(rgb_led, pulse_ns);

    if (err) {
        printk("%s: pwm_set failed (%d)\n", name, err);
    }
}

void hilo_actuador_run(void *p1, void *p2, void *p3)
{
    struct rgb_pwm_t rgb_pwm = {0};
    // ONLY FOR feat/pwm
    struct rgb_pwm_t rgb_pwm_dummy = {0}; 
    uint8_t color_selector = 1;
    // END of only for feat/pwm
    static const struct gpio_dt_spec led_blinky = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
    static const struct pwm_dt_spec red = PWM_DT_SPEC_GET(DT_ALIAS(red_pwm_led));
    static const struct pwm_dt_spec green = PWM_DT_SPEC_GET(DT_ALIAS(green_pwm_led));
    static const struct pwm_dt_spec blue = PWM_DT_SPEC_GET(DT_ALIAS(blue_pwm_led));

    if (!pwm_is_ready_dt(&red) || !pwm_is_ready_dt(&green) || !pwm_is_ready_dt(&blue)) {
        printk("Error: PWM device not ready\n");
        return;
    }

    printk("    Actuator thread init   \n");

    while (1) {
        if (k_msgq_get(&q_rgb_data, &rgb_pwm_dummy, K_FOREVER) == 0) {
            /* Heartbeat: one toggle per message received */
            gpio_pin_toggle_dt(&led_blinky);

            /* Polarity (common anode/cathode) is handled by the DT flags, not here */

            // ONly for feat/pwm
            if(rgb_pwm.red == 0xff)
            {
                rgb_pwm.red = 0;
                color_selector <<= 1;
            }
            else if(color_selector == 1)
            {
                rgb_pwm.red++;
            }
            if(rgb_pwm.blue == 0xff)
            {
                rgb_pwm.blue = 0;
                color_selector <<= 1;
            }
            else if(color_selector == 2)

            {
                rgb_pwm.blue++;
            }
            if(rgb_pwm.green == 0xff)
            {
                rgb_pwm.green=0;
                color_selector = 1;
            }
            else if(color_selector == 4)
            {
                rgb_pwm.green++;
            }
            printk("RED: PWM %d\n", rgb_pwm.red);
            printk("BLUE: PWM %d\n", rgb_pwm.blue);
            printk("GREEN: PWM %d\n", rgb_pwm.green);


            // End of section for feat/pwm
            
            set_level(&red, "red", rgb_pwm.red);
            set_level(&green, "green", rgb_pwm.green);
            set_level(&blue, "blue", rgb_pwm.blue);
        }
    }
}

K_THREAD_DEFINE(hilo_actuador_id, STACK_SIZE, hilo_actuador_run,
                NULL, NULL, NULL, PRIORITY, 0, 0);
