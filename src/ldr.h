#ifndef LDR_H
#define LDR_H

#include <stdint.h>

int ldr_init(void);
int16_t ldr_read(void);   // porcentaje de luz (0..100) o errno negativo

#endif /* LDR_H */
