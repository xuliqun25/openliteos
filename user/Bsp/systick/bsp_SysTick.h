#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f4xx.h"

//�ڵδ�ʱ���жϷ������е���
void TimingDelay_Decrement(void);

// ��ʼ��ϵͳ�δ�ʱ��
void SysTick_Init(void);


//�ṩ��Ӧ�ó������
void Delay_ms(__IO u32 nTime);
#define Delay_10ms(x) Delay_ms(10*x)

#endif /* __SYSTICK_H */
