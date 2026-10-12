// Primero el propio header: así el compilador comprueba que las definiciones
// de este fichero coinciden con las declaraciones públicas.
#include "ldr.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/sys/printk.h>
#include <errno.h>
#include <zephyr/drivers/gpio.h>

#define ADC_RESOLUTION 12

// 1 = no apagar la alimentación de la LDR tras cada lectura (solo depuración)
#define LDR_ALWAYS_ON 0

static const struct adc_dt_spec ldr =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ldr);
static const struct gpio_dt_spec ldr_power =
    GPIO_DT_SPEC_GET(DT_ALIAS(ldrpower), gpios);

static int ldr_power_on(void) {
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
        return ret;
    }
    printk("LDR power turned on\n");
    return 0;
}

static int ldr_power_off(void) {
    printk("Turning off LDR power\n");
    // Liberar ADC después de la lectura
    int ret = pm_device_runtime_put(ldr.dev);
    if (ret < 0) {
        printk("ERROR: could not release ADC: %d\n", ret);
        return ret;
    }
#if LDR_ALWAYS_ON
    // Depuración: D2 se queda en alto para poder medir el divisor con el
    // multímetro.
    // El ADC sí se libera arriba, así que el runtime PM sigue equilibrado.
    printk("LDR power kept on (LDR_ALWAYS_ON)\n");
#else
    // Apagar LDR
    gpio_pin_set_dt(&ldr_power, 0);
    printk("LDR power turned off\n");
#endif
    return 0;
}

int ldr_init(void) {
    printk("Initializing LDR\n");
    if (!gpio_is_ready_dt(&ldr_power)) {
        printk("ERROR: GPIO LDR not ready\n");
        return -1;
    }

    int ret;
    // Configurar GPIO para alimentar LDR
    ret = gpio_pin_configure_dt(&ldr_power, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR GPIO: %d\n", ret);
        return -ENODEV;
    }

    if (!adc_is_ready_dt(&ldr)) {
        printk("ERROR: ADC not ready\n");
        return -ENODEV;
    }

    // Configurar el canal mientras el ADC está activo
    ret = adc_channel_setup_dt(&ldr);
    if (ret < 0) {
        printk("ERROR: could not configure ADC: %d\n", ret);
        return -ENODEV;
    }

    // Habilitar Runtime PM para el dispositivo ADC
    ret = pm_device_runtime_enable(ldr.dev);
    if (ret < 0) {
        printk("ERROR: could not enable Runtime PM: %d\n", ret);
        return ret;
    }
    
    printk("LDR initialization complete\n");
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
        printk("ERROR: could not turn on LDR power: %d\n", ret);
        return ret;
    }

    printk("Reading LDR value\n");
    uint16_t sample;
    struct adc_sequence sequence = {
        .buffer = &sample,
        .buffer_size = sizeof(sample),
        .resolution = ADC_RESOLUTION,
    };
    ret = adc_sequence_init_dt(&ldr, &sequence);
    if (ret < 0) {
        printk("ERROR: could not initialize sequence: %d\n", ret);
    } else {
        ret = adc_read_dt(&ldr, &sequence);
        if (ret < 0) {
            printk("ERROR: ADC read failed: %d\n", ret);
        }
    }

    ret = ldr_power_off();
    if (ret < 0) {
        printk("ERROR: could not turn off LDR power: %d\n", off_ret);
    }

    if (ret < 0) {
        return ret;
    }
    printk("LDR read complete: %d\n", sample);

    *percentage = raw_to_percentage(sample);
    return 0;
}
