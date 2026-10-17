// Primero el propio header: así el compilador comprueba que las definiciones
// de este fichero coinciden con las declaraciones públicas.
#include "ldr.h"
#include "gpio.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/printk.h>
#include <errno.h>
#include <zephyr/drivers/gpio.h>

#define ADC_RESOLUTION 12

/*
 * Define whether the LDR should always remain powered on or not
 * (1 = always on for debugging purposes, 0 = power off after each read)
 */
#define LDR_ALWAYS_ON 0

LOG_MODULE_REGISTER(ldr, LOG_LEVEL_INF);

static const struct adc_dt_spec ldr =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ldr);
static const struct gpio_dt_spec ldr_power =
    GPIO_DT_SPEC_GET(DT_ALIAS(ldrpower), gpios);

static int ldr_power_on(void) {
    LOG_DBG("Turning on LDR power\n");
    int ret = gpio_power_on(&ldr_power);
    if (ret < 0) {
        LOG_ERR("could not activate LDR power: %d\n", ret);
        return ret;
    }
    // Wait for the voltage to stabilize before reading.
    k_busy_wait(1000);
    // Ensure the ADC is active before attempting to read from it.
    ret = pm_device_runtime_get(ldr.dev);
    if (ret < 0) {
        LOG_ERR("could not reactivate ADC: %d\n", ret);
        // Turn off the LDR power if the ADC could not be reactivated.
        gpio_power_off(&ldr_power);
        return ret;
    }
    LOG_DBG("LDR power turned on\n");
    return 0;
}

static int ldr_power_off(void) {
    LOG_DBG("Turning off LDR power\n");
    // Release the ADC device before turning off the LDR power.
    int ret = pm_device_runtime_put(ldr.dev);
    if (ret < 0) {
        LOG_ERR("could not release ADC: %d\n", ret);
        return ret;
    }
#if LDR_ALWAYS_ON
    LOG_DBG("LDR power kept on (LDR_ALWAYS_ON)\n");
#else
    // Turn off the LDR power.
    ret = gpio_power_off(&ldr_power);
    if (ret < 0) {
        LOG_ERR("could not deactivate LDR power: %d\n", ret);
        return ret;
    }
#endif
    return 0;
}

int ldr_init(void) {
    LOG_INF("init\n");
    int ret;
    // Initialize the GPIO that powers the LDR.
    ret = gpio_output_init(&ldr_power);
    if (ret < 0) return ret;

    if (!adc_is_ready_dt(&ldr)) {
        LOG_ERR("ADC not ready\n");
        return -1;
    }

    // Set up the ADC channel while the ADC is active.
    ret = adc_channel_setup_dt(&ldr);
    if (ret < 0) {
        LOG_ERR("could not configure ADC: %d\n", ret);
        return ret;
    }

    // Enable Runtime PM for the ADC device.
    ret = pm_device_runtime_enable(ldr.dev);
    if (ret < 0) {
        LOG_ERR("could not enable Runtime PM: %d\n", ret);
        return ret;
    }
    
    LOG_INF("init complete\n");
    return 0;
}

static uint8_t raw_to_percentage(uint16_t raw) {
    return (raw * 100) / ((1 << ADC_RESOLUTION) - 1);
}

int ldr_read(uint8_t *percentage) {
    if (percentage == NULL) {
        return -EINVAL;
    }

    int ret = ldr_power_on();
    if (ret < 0) {
        LOG_ERR("could not turn on LDR power: %d\n", ret);
        return ret;
    }

    LOG_DBG("Reading LDR value\n");
    uint16_t sample;
    struct adc_sequence sequence = {
        .buffer = &sample,
        .buffer_size = sizeof(sample),
        .resolution = ADC_RESOLUTION,
    };
    ret = adc_sequence_init_dt(&ldr, &sequence);
    if (ret < 0) {
        LOG_ERR("could not initialize sequence: %d\n", ret);
    } else {
        ret = adc_read_dt(&ldr, &sequence);
        if (ret < 0) {
            LOG_ERR("ADC read failed: %d\n", ret);
        }
    }

    int ret_off = ldr_power_off();
    if (ret < 0) return ret;
    if (ret_off < 0) {
        LOG_ERR("could not turn off LDR power: %d\n", ret_off);
        return ret_off;
    }

    LOG_DBG("LDR read complete: %d\n", sample);

    *percentage = raw_to_percentage(sample);
    return 0;
}
