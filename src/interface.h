
#ifndef INTERFACE_H
#define INTERFACE_H

#include <zephyr/kernel.h>

/* 1. Estructuras de datos que viajan entre hilos */
typedef struct
{
    uint16_t brightness;
} ldr_t;

typedef struct
{
    bool blue;
    bool red;
    bool green;
} rgb_t;

typedef enum
{
    NO_PRESSED,
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS,
} button_type_of_event_t;

/*
Read https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html#defining-a-message-queue
*/
#define MAX_NUMBER_OF_MESSAGES 10
extern struct k_msgq q_ldr_data;
extern struct k_msgq q_rgb_data;
extern struct k_msgq q_button_data;

// Event to signal de ADC to stop
#define EV_ADC_ON BIT(0)
extern struct k_event e_enable_devices;

#endif /* INTERFACE_H */
