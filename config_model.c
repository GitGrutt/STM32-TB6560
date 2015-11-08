#include "config_model.h"
#include "st_flashdriver.h"

/*
����ѡ�
#define MIN_PULSE_NUM               50  // ��ĳ���ٶ��±��ֵ���С������
#define DRIVERSFRACTION             1   //������ϸ����  
#define MOTOFRACTION                200 //���ϸ����  
#define MOTOMAXSTART                200 //��������ٶȣ����ٶȳ�����ֵ��Ե������˺�����λת/min   
#define MOTOMAXSPEED                600 //������ת�٣���λת/min (���2400ת/min)
#define ACCELE_TIM		    500 // ������ʱ��(MS)Acceleration time �ڸ�ʱ���ڼ�������Ƴ���������
#define ACCELE_PERTIM	            10// ���ٵ�λʱ�䣨MS��(���ݵ�λʱ���������ٶ��ݶ�) ���25MS

//ÿ����Ƶֵ���ã���8����
#define FRE_MULT_0		    1
#define FRE_MULT_1		    1  
#define FRE_MULT_2		    10
#define FRE_MULT_3		    50
#define FRE_MULT_4		    100
#define FRE_MULT_5		    500
#define FRE_MULT_6		    1000
#define FRE_MULT_7		    5000
*/

//----------------------UART1 GPIOConfig-------------//
static void USART1_GPIOConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  USART_DeInit(USART1);
  
  /* Enable Usart GPIO clock A�ڼ����ܸ���*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
  
//  GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE); 
  
  // Configure USARTx_Tx as alternate function push-pull 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  // Configure USARTx_Rx as input floating 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*    USART Initialization    */
void USART1_Config(void)
{
  USART_InitTypeDef	USART_InitStructure;
  
  USART1_GPIOConfig();
  
  USART_InitStructure.USART_BaudRate= 19200;//;//
  USART_InitStructure.USART_WordLength=USART_WordLength_8b;
  USART_InitStructure.USART_StopBits=USART_StopBits_1;
  USART_InitStructure.USART_Parity=USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;
  USART_Init(USART1,&USART_InitStructure);
  /* Enable the USART Transmoit interrupt: this interrupt is generated when the
  USART1 transmit data register is empty */
//  USART_ITConfig(USART1, USART_IT_TC, DISABLE);
  /* Enable the USART Receive interrupt: this interrupt is generated when the
  USART1 receive data register is not empty */
  USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
  USART_Cmd(USART1,ENABLE);				//Enable USART1
}

//******************************************************************************
// NVIC����
//******************************************************************************
void USART1_NVIC_Configuration(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;
  
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_Init(&NVIC_InitStructure);
}

//int fputc(int ch, FILE *f)
//{
//  USART_SendData(USART1, (uint8_t) ch);
//  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
//  return ch;
//}

