#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

enum button_event {
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS,
};

int button_init(void);

#endif /* BUTTON_H */