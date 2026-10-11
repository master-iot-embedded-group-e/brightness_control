
#ifndef INTERFACE_H
#define INTERFACE_H

#include <zephyr/kernel.h>

/* 1. Estructuras de datos que viajan entre hilos */
struct ldr_t {
    float value_lux;
    uint32_t timestamp;
};

struct rgb_pwm_t {
    uint8_t blue;
    uint8_t red;
    uint8_t green;
};

/*
Read https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html#defining-a-message-queue
*/
#define MAX_NUMBER_OF_MESSAGES 10
extern struct k_msgq q_ldr_data;
extern struct k_msgq q_rgb_data;

#endif /* INTERFACE_H */
