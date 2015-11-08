#include "sys.h" 
#include "stdio.h"
//******************************************************************************
// SysTick���ó�ʼ��
//******************************************************************************
void SysTick_Config1(void)
{
#if 1
#define SystemFreq		(FCLK*1000000.0)    // ��λΪHz
#define TB_SysTick		(TIME_TB*1000)		// ��λΪuS,��ʾ����ʵ��һ��
  
  static INT32U ticks;
  
  ticks=(INT32U)((TB_SysTick/1000000.0)*SystemFreq);
  SysTick_Config(ticks);
#endif	
}

//******************************************************************************
// ʱ�����ó�ʼ��
//******************************************************************************
void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;
  /*
  RCC_AdjustHSICalibrationValue �����ڲ����پ���HSI��У׼ֵ
  RCC_ITConfig ʹ�ܻ���ʧ��ָ����RCC�ж�
  RCC_ClearFlag ���RCC�ĸ�λ��־λ
  RCC_GetITStatus ���ָ����RCC�жϷ������
  RCC_ClearITPendingBit ���RCC���жϴ�����λ
  */
  /* RCC system reset(for debug purpose) */
  // ʱ��ϵͳ��λ
  RCC_DeInit();
  
  // ʹ���ⲿ��8M����
  // �����ⲿ���پ���HSE��
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  
  // ʹ�ܻ���ʧ���ڲ����پ���HSI��
  RCC_HSICmd(DISABLE);
  
  // �ȴ�HSE����
  // �ú������ȴ�ֱ��HSE�����������ڳ�ʱ��������˳�
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  
  if(HSEStartUpStatus == SUCCESS)
  {
    // ����AHBʱ�ӣ�HCLK��
    RCC_HCLKConfig(RCC_HCLK_Div_);	// 36 MHz
    
    // ���õ���AHBʱ�ӣ�PCLK1��
    RCC_PCLK1Config(RCC_PCLK1_Div_);	// 2.25 MHz
    
    // ���ø���AHBʱ�ӣ�PCLK2��
    RCC_PCLK2Config(RCC_PCLK2_Div_);	// 2.25 MHz
    
    /* ADCCLK = PCLK2/8 */
    // ����ADCʱ�ӣ�ADCCLK��
    RCC_ADCCLKConfig(RCC_ADC_DIV_);	// 0.281Mhz
    
    // ����USBʱ�ӣ�USBCLK��
    // USBʱ�� = PLLʱ�ӳ���1.5
    //RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    
    // �����ⲿ���پ���LSE��
    RCC_LSEConfig(RCC_LSE_OFF);
    
    // ʹ�ܻ���ʧ���ڲ����پ���LSI��
    // LSE����OFF
    RCC_LSICmd(DISABLE);
    
    // ����RTCʱ�ӣ�RTCCLK��
    // ѡ��HSEʱ��Ƶ�ʳ���128��ΪRTCʱ��
    //RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
    
    // ʹ�ܻ���ʧ��RTCʱ��
    // RTCʱ�ӵ���״̬
    RCC_RTCCLKCmd(DISABLE);
    
    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
    
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    
    /* PLLCLK = 8MHz * 9 = 72 MHz */
    // ����PLLʱ��Դ����Ƶϵ��
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_); 
    
    /* Enable PLL */
    // ʹ�ܻ���ʧ��PLL
    RCC_PLLCmd(ENABLE);
    
    /* Wait till PLL is ready */
    // ���ָ����RCC��־λ�������
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }
    
    /* Select PLL as system clock source */
    // ����ϵͳʱ�ӣ�SYSCLK��
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    
    /* Wait till PLL is used as system clock source */
    // ��������ϵͳʱ�ӵ�ʱ��Դ
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
  
//  // ʹ�ܻ���ʧ��AHB����ʱ��
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1
//                        |RCC_AHBPeriph_DMA2
//                          |RCC_AHBPeriph_SRAM
//                            |RCC_AHBPeriph_FLITF
//                              |RCC_AHBPeriph_CRC
//                                |RCC_AHBPeriph_FSMC
//                                  |RCC_AHBPeriph_SDIO,DISABLE);
//  // ʹ�ܻ���ʧ��APB1����ʱ��
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_ALL,DISABLE);
//  
//  // ǿ�ƻ����ͷŸ���APB��APB2�����踴λ
//  RCC_APB2PeriphResetCmd(RCC_APB2Periph_ALL,ENABLE);
//  // �˳���λ״̬
//  RCC_APB2PeriphResetCmd(RCC_APB2Periph_ALL,DISABLE);
//  
//  // ǿ�ƻ����ͷŵ���APB��APB1�����踴λ
//  RCC_APB1PeriphResetCmd(RCC_APB1Periph_ALL,ENABLE);
//  
//  // ǿ�ƻ����ͷź���λ
//  RCC_BackupResetCmd(ENABLE);
//  
//  // ʹ�ܻ���ʧ��ʱ�Ӱ�ȫϵͳ
//  RCC_ClockSecuritySystemCmd(DISABLE);
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
}
//******************************************************************************
// NVIC����
//******************************************************************************
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel=TIM1_CC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  //NVIC_InitStructure.NVIC_IRQChannel=DMA1_Channel5_IRQn;
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel= EXTI2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel= EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

//******************************************************************************
// GPIO����
//******************************************************************************
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB
                         |RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
  
  //time1 set----------------------------------------------------------------
//  GPIO_Write(GPIOA,0xffff);
  /* GPIOA Configuration: Channel 3 as alternate function push-pull PA10/USART1_RX/TIM1_CH3*/
//  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
//  GPIO_SetBits(GPIOA,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);
  /*PA11 / USART1_CTS/CANRX / USBDM/TIM1_CH4*/
  
//  GPIO_Write(GPIOB,0xffff);	// 11111101-11111111 
//  /* GPIOB Configuration: Channel 3N as alternate function push-pull PB15/SPI2_MOSI/TIM1_CH3N*/
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  //LED-------------------------------------------------------------------------
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_6|GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(GPIOA,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_6|GPIO_Pin_5);
  
  //��Ƶ���� 1 10 100��λ ���ȼ��Ӹ�����----------------------------
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
  
  /*
  *     x-y-z-a Axis channel ����ѡ�� xyza��(�źŲ�׽) ���ȼ��Ӹ�����
  *     ����ڿ�ʼ����ǰ����׽��ЩIO���ϵ��ź�,�б����е�����һ�ᡣ
  *     Ϊ��ȷ����׽�ȶ��ԣ���Ҫ�ಶ׽���Ρ�
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  //x-y-z-a Axis channel ����ѡ�� xyza��(�ź���������������) �����ʹ��---------
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE); 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOB, GPIO_Pin_14);    // ����� ʹ�� �ܽ�  
  
  /*
  *     PA7 ���巽�򣨵������ת�� ���� ��IO���ڲ�׽����CPLD�ϵ�����ת�ź�
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  

 /*
  *     ���������źŲ�׽
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure); 

 /*
  *     ��ͣ��
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure); 
}

void ModeInquiryInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);
  
  //����ģʽ ���� ��ͨģʽ �л� ������ʱ ��ȡ��ȷ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint8_t GetModeInquiry(void)
{
  return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);
}

//******************************************************************************
// pcb�ϵ�ָʾ��
//******************************************************************************
void led_toggle(uint16_t led_pin_x)//��ת
{
  GPIOA->ODR ^= led_pin_x;
}
void led_off(uint16_t led_pin_x)// �ص� BSRR GPIO_SetBits
{
  GPIOA->BSRR = led_pin_x;
}
void led_on(uint16_t led_pin_x)// ���� BRR 
{
  GPIOA->BRR = led_pin_x;
}

//******************************************************************************
// ��ʱ���򣬵�λΪ*1ms
//******************************************************************************
void delayms(INT16U cnt)
{
  //#define  	CONST_1MS  7333  	// 72MhZ
  //#define  	CONST_1MS 3588  	// 32MhZ
#define  	CONST_1MS (105*FCLK) 
  
  INT16U i;
  
  __no_operation();
  while(cnt--)
    for (i=0; i<CONST_1MS; i++);
}
