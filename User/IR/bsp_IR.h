#ifndef __BSP_IR_H
#define __BSP_IR_H

#include "stm32f10x.h"

#define ReadGPA8 (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 1)        

/************************ �������� *************************/
 /*
  * ��  �ܣ�Infrared��ʼ��
  * ��  ����void
  * ����ֵ��void
  *
 */
void Infrared_Init(void);

void GetData(void);


#endif
