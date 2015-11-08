/**
  ******************************************************************************
  * @file    Project/Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.0.0
  * @date    04/06/2009
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

#include "stm32_m.h"
#include "stepper_motor.h"
#include  "usart1_reader.h"

#include <stdlib.h>
#include <string.h>

#define UC_TIME_CNT   50                // ��ʱʱ��
#define READY         0x00
#define RECEIVE       0x01    
    
extern uint8_t SpeedStatus;
extern uint8_t StypeStatus;
//extern uint8_t SpeedStatus;
extern volatile INT32U PulsesNumber;
extern INT32U FrequencyMultiplication;

uint8_t Tim1OverFlag=0;
volatile INT8U TimeTick;

/*����1 _ 232*/
static INT8U Uc1Buf[Q_USART1_ELEMENT];   
volatile static INT8U Uc1Len = 0;
volatile static INT8U Uc1TimeoutCnt;
volatile static INT8U Uc1_status = READY;;

//******************************************************************************
/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval : None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval : None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval : None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval : None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval : None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval : None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval : None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval : None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval : None
  */
void SysTick_Handler(void)
{
  TimeTick ++;
  
  if(Uc1TimeoutCnt)
  {
    Uc1TimeoutCnt ++;
    if(Uc1TimeoutCnt > UC_TIME_CNT)
    {
      memset(Uc1Buf,0,sizeof(Uc1Buf));
      Uc1Len = 0x00;
      Uc1TimeoutCnt = 0x00;
      Uc1_status = READY;
    }
  }
}

/* ���Ϊ��ʼ��״̬,��Ҫ��ȡ
 * �����ת����ѡ����˶��ἰ��Ƶ����
 * Ȼ����ݱ�Ƶ���������塣
 */
void EXTI2_IRQHandler(void)//EXTI9_5_IRQHandler
{
  INT32U volatile_tmp;
  
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {
    if(SPD_INIT == SpeedStatus)
    {
      MotorRunningEnable();
      IntFrequencyMAxisCapture();// ����ѡ�񼰱�Ƶ
//      memcpy((void*)&PulsesNumber,(void*)&FrequencyMultiplication,sizeof(PulsesNumber));
      PulsesNumber += FrequencyMultiplication;//��Ƶϵ�� 1,10,100
//      PulsesNumber = FrequencyMultiplication;//��Ƶϵ�� 1,10,100
      Tim1_Configuration(IntSpeedCalculation());
//      TIM_CtrlPWMOutputs(TIM1, ENABLE);             // ����������Ҫ��!!!
      EXTI_ClearITPendingBit(EXTI_Line2);
      return;
    }
    volatile_tmp = PulsesNumber;
    if(volatile_tmp < (volatile_tmp + FrequencyMultiplication))//����� ѡ�ж�ʧ����
    {
      PulsesNumber += FrequencyMultiplication;//��Ƶϵ�� 1,10,100
    }
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}


void EXTI0_IRQHandler(void)//EXTI9_5_IRQHandler
{  
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    StopTimmer();
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}


/******************************************************************************
 * ����Ҫ�ж��Ƿ�Ӧ�ñ��ֵ�ǰ���˶�״̬
 * ����Ҫ�ж��Ƿ�Ӧ�ñ��ֵ�ǰ���˶�״̬,�������Ҫ����Ҫ�жϵ�ǰ�Ǽ��ٻ��Ǽ���
******************************************************************************/
void TIM1_UP_IRQHandler(void)		// ʵ�ʲ���������Ϊ50US
{
  
  if(TIM_GetITStatus(TIM1,TIM_IT_Update) != RESET)
  {
    Tim1_Configuration(IntSpeedCalculation());//���ݼӼ���״̬��������Ӧ�Ĳ���
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  }
}

/******************************************************************************/
void TIM1_CC_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC3);	
}

/******************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{
    DMA_ClearITPendingBit(DMA1_IT_TC5);
//    GPIOB->ODR ^= GPIO_Pin_7;		// for test!
 }

void USART1_IRQHandler(void)
{
  INT8U tmp_data;
  
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//����Ĵ�����������
  {
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    
    tmp_data = USART_ReceiveData(USART1);
    switch(Uc1_status)
    {
    case READY:
      if(tmp_data == EX_AD_DOMAIN_EN) // ���տ�ʼ
      {
        memset(Uc1Buf,0,sizeof(Uc1Buf));
        Uc1Len = 0x00;
        Uc1TimeoutCnt = 1;    // ��ʼ��ʱ����,30ms��ʱ
        Uc1_status = RECEIVE;
      }
      else
      {
        break;
      }
    case RECEIVE:
      Uc1Buf[Uc1Len++] = tmp_data;
      if(Uc1Len > Q_USART1_ELEMENT)
      {
        memset(Uc1Buf,0,sizeof(Uc1Buf));
        Uc1Len = 0x00;
        Uc1TimeoutCnt = 0;
        Uc1_status = READY;
        break;
      }
      if((tmp_data == EX_DATA_EX)&&(Uc1Buf[5] == Uc1Len-8)) // ��������
      {
        Usart1EventEnqueue(Uc1Buf);
        Uc1_status = READY;
        memset(Uc1Buf,0,sizeof(Uc1Buf));
        Uc1Len = 0x00;
        Uc1TimeoutCnt = 0;
        Uc1_status = READY;
      }
      break;
    default:
      break;
    }
  }
}

//******************************************************************************
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
//******************************************************************************
