#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "rgb_pwm.h"
#include "interface.h"

#define STACK_SIZE 1024
#define PRIORITY 7

// Demo function to cycle through RGB PWM values. Delete this in production.
static void pwm_demo(struct rgb_pwm_t *rgb_pwm, uint8_t *color_selector) {
    if(rgb_pwm->red == 0xff)
    {
        rgb_pwm->red = 0;
        *color_selector <<= 1;
        printk("RED: PWM reset to 0\n");
    }
    else if(*color_selector == 1)
    {
        rgb_pwm->red++;
    }
    if(rgb_pwm->blue == 0xff)
    {
        rgb_pwm->blue = 0;
        *color_selector <<= 1;
        printk("BLUE: PWM reset to 0\n");
    }
    else if(*color_selector == 2)

    {
        rgb_pwm->blue++;
    }
    if(rgb_pwm->green == 0xff)
    {
        rgb_pwm->green=0;
        *color_selector = 1;
        printk("GREEN: PWM reset to 0\n");
    }
    else if(*color_selector == 4)
    {
        rgb_pwm->green++;
    }
}

void hilo_actuador_run(void *p1, void *p2, void *p3)
{
    struct rgb_pwm_t rgb_pwm = {0};
    // ONLY FOR feat/pwm
    struct rgb_pwm_t rgb_pwm_dummy = {0};
    uint8_t color_selector = 1;

    printk("    Actuator thread init   \n");

    while (1) {
        if (k_msgq_get(&q_rgb_data, &rgb_pwm_dummy, K_FOREVER) == 0) {

            /* Polarity (common anode/cathode) is handled by the DT flags, not here */
            check_pwm_ready();
            // Demo function to cycle through RGB PWM values. Delete this in production.
            pwm_demo(&rgb_pwm, &color_selector); 
            // End of section for feat/pwm
            
            set_red_level(rgb_pwm.red);
            set_green_level(rgb_pwm.green);
            set_blue_level(rgb_pwm.blue);
        }
    }
}

K_THREAD_DEFINE(hilo_actuador_id, STACK_SIZE, hilo_actuador_run,
                NULL, NULL, NULL, PRIORITY, 0, 0);
