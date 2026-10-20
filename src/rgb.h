#ifndef RGB_H
#define RGB_H

#include <stdbool.h>
#include <stdint.h>

int rgb_init(void);
int red_on(void);
int red_off(void);
int green_on(void);
int green_off(void);
int blue_on(void);
int blue_off(void);

/* Drive the three channels at once. Returns the first error, 0 on success. */
int rgb_set(bool red, bool green, bool blue);

#endif /* RGB_H */
