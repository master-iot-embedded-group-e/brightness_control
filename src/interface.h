
#ifndef INTERFACE_H
#define INTERFACE_H

#include <zephyr/kernel.h>

/* 1. Estructuras de datos que viajan entre hilos */
typedef struct {
    uint16_t brightness;
    uint32_t timestamp;
} ldr_t;

typedef struct {
    bool blue;
    bool red;
    bool green;
} rgb_t;

typedef enum{
    SHORT,
    LONG
} button_press_duration_t;

typedef struct {
    button_press_duration_t duration;
} button_press_t;

typedef struct actuator_data_t{
    button_press_t button_press;
    rgb_t rgb;
};

/*
Read https://docs.zephyrproject.org/latest/kernel/services/data_passing/message_queues.html#defining-a-message-queue
*/
#define MAX_NUMBER_OF_MESSAGES 10
extern struct k_msgq q_ldr_data;
extern struct k_msgq q_rgb_data;
extern struct k_msgq q_sensor_data;

#endif /* INTERFACE_H */
