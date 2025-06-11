#ifndef _TIMER4_H
#define _TIMER4_H

extern char timer4_flag;   //外部变量声明，1:定时器4中断，有数据到

void Tim4_Init(uint16_t arr, uint16_t psc);

#endif
