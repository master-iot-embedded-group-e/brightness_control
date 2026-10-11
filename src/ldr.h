#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/printk.h>
#include <errno.h>
#include <zephyr/drivers/gpio.h>

#define ADC_RESOLUTION 12

static const struct adc_dt_spec ldr =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ldr);
static const struct gpio_dt_spec ldr_power =
    GPIO_DT_SPEC_GET(DT_ALIAS(ldrpower), gpios);

void ldr_power_on(void) {
    printk("Turning on LDR power\n");
    gpio_pin_set_dt(&ldr_power, 1);
    // Tiempo de estabilización de LDR (1ms)
    k_busy_wait(1000);
    // Reactivar ADC antes de utilizarlo
    int ret = pm_device_runtime_get(ldr.dev);
    if (ret < 0) {
        printk("ERROR: could not reactivate ADC: %d\n", ret);
        // Apagar LDR si no se pudo reactivar el ADC
        gpio_pin_set_dt(&ldr_power, 0);
        return;
    }
    printk("LDR power turned on\n");
}

void ldr_power_off(void) {
    printk("Turning off LDR power\n");
    // Liberar ADC después de la lectura
    int ret = pm_device_runtime_put(ldr.dev);
    if (ret < 0) {
        printk("ERROR: could not release ADC: %d\n", ret);
    }
    // Apagar LDR
    gpio_pin_set_dt(&ldr_power, 0);
    printk("LDR power turned off\n");
}

void ldr_init(void) {
    printk("Initializing LDR\n");
    if (!gpio_is_ready_dt(&ldr_power)) {
        printk("ERROR: GPIO LDR not ready\n");
        return;
    }

    // Configurar GPIO para alimentar LDR
    int ret = gpio_pin_configure_dt(&ldr_power, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR GPIO: %d\n", ret);
        return;
    }

    if (!adc_is_ready_dt(&ldr)) {
        printk("ERROR: ADC not ready\n");
        return;
    }

    // Configurar el canal mientras el ADC está activo
    ret = adc_channel_setup_dt(&ldr);
    if (ret < 0) {
        printk("ERROR: could not configure ADC: %d\n", ret);
        return;
    }

    // Habilitar Runtime PM para el dispositivo ADC
    ret = pm_device_runtime_enable(ldr.dev);
    if (ret < 0) {
        printk("ERROR: could not enable Runtime PM: %d\n", ret);
        return;
    }
    
    printk("LDR initialization complete\n");
}

uint16_t ldr_read(void) {
    ldr_power_on();
    printk("Reading LDR value\n");
    int ret;
    uint16_t sample;
    struct adc_sequence sequence = {
        .buffer = &sample,
        .buffer_size = sizeof(sample),
        .resolution = ADC_RESOLUTION,
    };
    // Obtener lectura
    ret = adc_sequence_init_dt(&ldr, &sequence);
    if (ret == 0) {
        ret = adc_read_dt(&ldr, &sequence);
    } else {
        printk("ERROR: could not initialize sequence: %d\n", ret);
    }
    printk("LDR read complete: %d\n", sample);
    ldr_power_off();
    return sample;
}

uint8_t raw_to_percentage(uint16_t raw) {
    return (raw * 100) / ((1 << ADC_RESOLUTION) - 1);
}
