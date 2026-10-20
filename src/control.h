#ifndef CONTROL_H
#define CONTROL_H

/*
 * Runs the control FSM in the calling thread (main). It never returns unless
 * initialization fails, in which case it returns the negative error code.
 */
int control_run(void);

#endif /* CONTROL_H */
