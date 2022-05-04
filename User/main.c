
// ������ʱ��TIMx,x[6,7]��ʱӦ��
#include "stm32f10x.h"
#include "sys.h"
#include "bsp_led.h"
#include "bsp_GeneralTim.h"
//#include "bsp_IR.h"
#include "bsp_usart_dma.h"


volatile uint32_t time = 0; // ms ��ʱ���� 

extern uint8_t Remote;

int main(void)
{
	Systick_Init();
	
  GENERAL_TIM_Init(); 
	
	USART_Config();
	
	printf("��ʼ���ɹ�����ʼ�����⣡\r\n");
	
	while(1){
		if( Remote != 0 )              //�������������ֵ������յ�ң�ذ����ˣ������ң�ذ���ֵΪ0����Ҫ��Remote�ĳ�ʼֵ��������ж���  
        {
            Remote_Scan(Remote);     //ң�ذ�������
        }  
		
	}

}
/*********************************************END OF FILE**********************/

