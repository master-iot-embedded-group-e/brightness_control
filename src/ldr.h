#ifndef LDR_H
#define LDR_H

#include <stdint.h>

int ldr_init(void);
int ldr_read(uint8_t *percentage);

#endif /* LDR_H */
