#ifndef __READPWM_H__
#define __READPWM_H__

/* include statements */
#include "main.h"

/* macro definitions */

/* struct/enum/union definitions */

extern float period;
extern int flag_PWM, flag_PWM_out;

extern volatile int flag_writeSD;
extern String pwmStr;
extern char bufferFloat[20];
extern bool outputSent;

/* function prototypes */
void readpwm();

#endif /* __READPWM_H__ */
