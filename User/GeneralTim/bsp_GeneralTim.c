#include "bsp_GeneralTim.h"
#include "bsp_usart_dma.h"

u8  IRdatas = 0;                //���յ����ݴ���
u8  IRval  = 0;                 //���յ��İ���ֵ           
u8  RmtCnt = 0;                 //�����ظ����µĴ���
u8  Remote = 0;                 //ң�ذ�������ֵ��ȫ�ֱ���
//u8   i;

// ��ʱ�����벶���û��Զ�������ṹ�嶨��
TIM_ICUserValueTypeDef TIM_ICUserValueStructure = {0,0,0,0};

// �ж����ȼ�����
static void GENERAL_TIM_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    // �����ж���Ϊ0
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);		
		// �����ж���Դ
    NVIC_InitStructure.NVIC_IRQChannel = GENERAL_TIM_IRQ;	
		// ���������ȼ�Ϊ 0
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	 
	  // ������ռ���ȼ�Ϊ3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void GENERAL_TIM_GPIO_Config(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // ���벶��ͨ�� GPIO ��ʼ��
	RCC_APB2PeriphClockCmd(GENERAL_TIM_CH1_GPIO_CLK, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GENERAL_TIM_CH1_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        //��������
  GPIO_Init(GENERAL_TIM_CH1_PORT, &GPIO_InitStructure);	
}


///*
// * ע�⣺TIM_TimeBaseInitTypeDef�ṹ��������5����Ա��TIM6��TIM7�ļĴ�������ֻ��
// * TIM_Prescaler��TIM_Capture_Period������ʹ��TIM6��TIM7��ʱ��ֻ���ʼ����������Ա���ɣ�
// * ����������Ա��ͨ�ö�ʱ���͸߼���ʱ������.
// *-----------------------------------------------------------------------------
// *typedef struct
// *{ TIM_Prescaler            ����
// *	TIM_CounterMode			     TIMx,x[6,7]û�У���������
// *  TIM_Capture_Period               ����
// *  TIM_ClockDivision        TIMx,x[6,7]û�У���������
// *  TIM_RepetitionCounter    TIMx,x[1,8,15,16,17]����
// *}TIM_TimeBaseInitTypeDef; 
// *-----------------------------------------------------------------------------
// */

/* ----------------   PWM�ź� ���ں�ռ�ձȵļ���--------------- */
// ARR ���Զ���װ�ؼĴ�����ֵ
// CLK_cnt����������ʱ�ӣ����� Fck_int / (psc+1) = 72M/(psc+1)
// PWM �źŵ����� T = ARR * (1/CLK_cnt) = ARR*(PSC+1) / 72M
// ռ�ձ�P=CCR/(ARR+1)

static void GENERAL_TIM_Mode_Config(void)
{  
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  // ������ʱ��ʱ��,���ڲ�ʱ��CK_INT=72M
	GENERAL_TIM_APBxClock_FUN(GENERAL_TIM_CLK,ENABLE);        //ʹ��TIMxʱ��

/*--------------------ʱ���ṹ���ʼ��-------------------------*/	
  
	// �Զ���װ�ؼĴ�����ֵ���ۼ�TIM_Capture_Period+1��Ƶ�ʺ����һ�����»����ж�  ��10ms��
	TIM_TimeBaseStructure.TIM_Period=GENERAL_TIM_PERIOD;	
	// ����CNT��������ʱ�� = Fck_int/(psc+1)
	TIM_TimeBaseStructure.TIM_Prescaler= GENERAL_TIM_PSC;	
	// ʱ�ӷ�Ƶ���� ����������ʱ��ʱ��Ҫ�õ�
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		
	// ����������ģʽ������Ϊ���ϼ���
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;		
	// �ظ���������ֵ��û�õ����ù�
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;	
	// ��ʼ����ʱ��
	TIM_TimeBaseInit(GENERAL_TIM, &TIM_TimeBaseStructure);

	/*--------------------���벶��ṹ���ʼ��-------------------*/	
	// �������벶���ͨ������Ҫ���ݾ����GPIO������
	TIM_ICInitStructure.TIM_Channel = GENERAL_TIM_CHANNEL_x;
	// ���벶���źŵļ�������
	TIM_ICInitStructure.TIM_ICPolarity = GENERAL_TIM_STRAT_ICPolarity;
	// ����ͨ���Ͳ���ͨ����ӳ���ϵ����ֱ���ͷ�ֱ������
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	// �������Ҫ��������źŵķ�Ƶϵ��
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	// �������Ҫ��������źŵ��˲�ϵ��
	TIM_ICInitStructure.TIM_ICFilter = 0;
	// ��ʱ�����벶���ʼ��
	TIM_ICInit(GENERAL_TIM, &TIM_ICInitStructure);
	
	// ������ºͲ����жϱ�־λ
  TIM_ClearFlag(GENERAL_TIM, TIM_FLAG_Update|GENERAL_TIM_IT_CCx);	
  // �������ºͲ����ж�  
	TIM_ITConfig (GENERAL_TIM, TIM_IT_Update | GENERAL_TIM_IT_CCx, ENABLE );
	
	// ʹ�ܼ�����
	TIM_Cmd(GENERAL_TIM, ENABLE);
}

void GENERAL_TIM_Init(void)
{
	GENERAL_TIM_GPIO_Config();
	GENERAL_TIM_NVIC_Config();
	GENERAL_TIM_Mode_Config();		
}



TIM_ICUserValueTypeDef TIM_Values;    //��ʼ���ṹ�� 

void GENERAL_TIM_INT_FUN(void)                              //��ʱ��x�жϷ������
{
    if ( TIM_GetITStatus( GENERAL_TIM, TIM_IT_Update ) != RESET )
    {
        if ( TIM_Values.DataFlag == 1 )                 //�Ƿ���յ���������
        {
            if( TIM_Values.Capture_Period > 3 )                 //���4������жϣ�40ms��û�յ����ݣ���Ҫô��ʼ���ظ��룬Ҫô����������
            {
                if( RmtCnt == 0 || TIM_Values.Capture_Period > 9 )                          //����յ����쵼�룬����4������м�û���յ��ظ��룬���ж��������ݣ���0��־�˳�                                                       
                    TIM_Values.DataFlag = 0;                                                //�����յ��ظ��룬���м�90msû�ٴ����жϣ����ж��������ݣ���0�˳�
            }
            TIM_Values.Capture_Period++;
        }
    } 
    if ( TIM_GetITStatus( GENERAL_TIM, GENERAL_TIM_IT_CCx ) != RESET )                              //�����������ػ����½����¼�?
    {
        if ( TIM_Values.Capture_StartFlag == 0 )                            //��һ�������ز���
        {   
            TIM_SetCounter( GENERAL_TIM, 0 );                               //��0������
            GENERAL_TIM_OCxPolarityConfig_FUN( GENERAL_TIM, TIM_ICPolarity_Falling );       //����Ϊ�½��ز���
            TIM_Values.Capture_CcrValue = 0;                //����ֵ��0             
            TIM_Values.Capture_StartFlag = 1;               //��ʼ�½��ز���
            TIM_Values.Capture_Period = 0;              //�Զ���װ�ؼĴ�����0
        }
        else                //�ڶ��β����½��ز���
        {
            TIM_Values.Capture_CcrValue = GENERAL_TIM_GetCapturex_FUN( GENERAL_TIM );           //��ȡͨ��4 ����Ĵ�����ֵ
            GENERAL_TIM_OCxPolarityConfig_FUN( GENERAL_TIM, TIM_ICPolarity_Rising );                        //����Ϊ�����ز���
            TIM_Values.Capture_StartFlag = 0;                                   //��ʼ��־��0�������жϲ���������
            TIM_Values.Capture_FinishFlag = 1;                                  //������1�β�������
 
            if ( TIM_Values.Capture_FinishFlag == 1 )                           //�ж��Ƿ����һ�β�������
            {
                if ( TIM_Values.DataFlag == 1 )                                     //�Ƿ���յ���������
                {
                    if ( TIM_Values.Capture_CcrValue > 300 && TIM_Values.Capture_CcrValue < 800 )           //560Ϊ��׼ֵ,560us
                    {
                        TIM_Values.Data <<= 1;                  //����һλ
                        TIM_Values.Data |= 0;                   //���յ�0
                        IRdatas++;                      //���յ������ݴ�����1.
                    }
                    else if ( TIM_Values.Capture_CcrValue > 1400 && TIM_Values.Capture_CcrValue < 1800 )    //1680Ϊ��׼ֵ,1680us
                    {
                        TIM_Values.Data <<= 1;                  //����һλ
                        TIM_Values.Data |= 1;                   //���յ�1											   
                        IRdatas++;              //���յ������ݴ�����1
                    }
										          
                    /*���� NEC ��涨��110ms������(�� 9ms �͵�ƽ+2.5m �ߵ�ƽ+0.56ms �͵�ƽ
                    +97.94ms �ߵ�ƽ���)�������һ֡���ݷ������֮�󣬰�����Ȼû�зſ��������ظ��룬
                    �������룬����ͨ��ͳ��������Ĵ�������ǰ������µĳ���/������������ ��0.56ms �ߵ�ƽ*/
 
                    else if ( TIM_Values.Capture_CcrValue > 2100 && TIM_Values.Capture_CcrValue < 2500 )    //�õ�������ֵ���ӵ���Ϣ 2250Ϊ��׼ֵ2.25ms
                    {
                        if( RmtCnt > 3 )            //��ֹ�ɿ��������յ��ظ��룬������ǰ3���յ����ظ��� 
                            IRdatas++;              //���յ������ݴ�����1
                        RmtCnt++;                   //������������1��  
                    }  
										
//										i  =  TIM_Values.Data;
//										printf ("%d",i);
                }   
                else if ( TIM_Values.Capture_CcrValue > 4200 && TIM_Values.Capture_CcrValue < 4700 )        //4500Ϊ��׼ֵ4.5ms
                {
                    TIM_Values.DataFlag = 1;            //�ɹ����յ���������,���ݿ�ʼ��־Ϊ1
                    IRdatas = 0;                        //����1ʱ�յ�������
                    RmtCnt = 0;                         //�����������������
                    
                }
            }
					}
				}
		
		 switch ( IRdatas )      
            {
//              case 8:     //������������
//                  if( TIM_Values.Data != IR_ID )                      //������յ����������ʹ��ң��������ͬ,������ȫ����0����
//                  {
//                      IRdatas = 0;                    //�������ݴ�����0
//                      TIM_Values.DataFlag = 0;        //��ʼ�������ݱ�־��λΪ0  
//                  }
//                  TIM_Values.Data = 0;                //���յ���������0��ֻ��Ҫ������
//                  break;
 
//                case 16:    //��������������
//                    //if ( (u8)~TIM_Values.Data != IR_ID )                  //���֪��ң��ID����ֻ���ض�ң�������ƣ��������������һ��
//                    if ( (u8)(TIM_Values.Data>>8) != (u8)~(TIM_Values.Data&0xff) )      //�����������յ����������룬�ȽϷ��ֲ�ͬ��������ȫ����0����
//                    {
//                        IRdatas = 0;                    //�������ݴ�����0
//                        TIM_Values.DataFlag = 0;        //��ʼ�������ݱ�־��λΪ0
//                    }
//                    TIM_Values.Data = 0;                //���յ���������0
//                    break;
 
                case 24:    //������������
                    IRval = TIM_Values.Data;        //��������浽IRval
                    TIM_Values.Data = 0;            //���յ���������0��׼���������ݷ���
								    Remote = IRval;
                    break;
								
 
//                case 32:    //���������ݷ���
//                    if ( IRval != (u8)~TIM_Values.Data )        //���������ͽ��յ������ݷ��벻ͬ������������0,���¿�ʼ
//                    {
//                        IRdatas = 0;                    //�������ݴ�����0
//                        IRval = 0;                      
//                    }
//                    TIM_Values.Data = 0;            //���յ���������0��׼��������һ���ж�����
//                    Remote = IRval;          //���յ��İ���ֵ��ֵ��ȫ�ֱ��� Remote
//                    IRdatas = 33;            //��ֵΪ33��ֹ�ڽ�����ʱ�ٽ����жϣ�����32���жϣ�����������0
//                    break;
//                    
//                case 34:    //�ظ���,�����x���ظ������յ�1���ظ��������Ͱ�34��x                  
//                    Remote = IRval;
//                    IRdatas = 33;            //���¸�ֵ��33����ֹ�ظ���������̫�࣬������ֵ������Ҳ���Ҫд�����switchѡ��
//                    break;
				
            } 
////				printf("%d",IRval);
										
				TIM_ClearFlag(GENERAL_TIM, TIM_FLAG_Update|GENERAL_TIM_IT_CCx);	       //����жϱ�־λ

}


//��ֲʱ���ݲ�ͬ��ң�ذ���ֵ������case
void Remote_Scan( uint8_t str )              //ң�ذ�������
{        
//    Remote = 0;
    switch( str )
    {
            case 0XA2:
						{
//                printf ("1\r\n");
						}
                break;
            case 0X68:
                //
                break;
             case 0X62:
						 {
                printf ("2\r\n");
						 }
                break;						
            case 0X30:
                //LCD_ShowString( 25, 55,"I MISS YOU" );  // ��ʾ�ַ���
                break;
            case 0X18:
						 {
                printf ("1\r\n");
						 }
                //LCD_ShowString( 30, 70,"I SEE YOU" );  // ��ʾ�ַ���
                break;
            case 0x7A:
                //LCD_Image( 0, 0, 240, 135, imageLoge );
                break;
            case 0x10:
                //LCD_Image( 100, 140, 60, 58, imageLoge2 );
                break;
            case 0x38:
                //LCD_ShowNum( 100, 55, time, sizeof(time)-1 );
                //time++;
                break;
            case 0x5A:
                //LED_TOGGLE( 2 );
                break;
            case 0x42:
                //LED_GPIO_Confing();
                //time = 0;
                //LCD_ShowNum( 100, 55, time, sizeof(time)-1 );
                break;            
    }
}

/*********************************************END OF FILE**********************/
