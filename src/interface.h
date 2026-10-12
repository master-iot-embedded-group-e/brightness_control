
#ifndef INTERFACE_H
#define INTERFACE_H

#include <zephyr/kernel.h>
#include "button.h"


/* 1. Estructuras de datos que viajan entre hilos */
struct ldr_t {
    uint16_t brightness;
    uint32_t timestamp;
};

struct rgb_t {
    bool blue;
    bool red;
    bool green;
};

struct button_events_t {
    enum button_event event;
    uint32_t timestamp;
};

/*
Read https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html#defining-a-message-queue
*/
#define MAX_NUMBER_OF_MESSAGES 10
extern struct k_msgq q_ldr_data;
extern struct k_msgq q_rgb_data;
extern struct k_msgq q_button_data;

#endif /* INTERFACE_H */
