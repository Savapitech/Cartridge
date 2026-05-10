#ifndef TRANSITIONS_H
#define TRANSITIONS_H

#include <stdint.h>

void transition_wipe_down(void);
void transition_wipe_up(void);
void transition_flash(uint8_t times);
void transition_shake(void);

#endif
