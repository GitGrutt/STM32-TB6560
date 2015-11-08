#include "stepper_motor.h"
#include "st_flashdriver.h"

extern uint32_t NOR_FlashWriteBuf[FLASH_MSG_LEN];
/*
ͨ���ڹ̶������ڸı�������������ﵽ�Ӽ��ٵ�Ч����
*/
/*************************************************************
* ��Щֵͨ����������ʱ�޸�
*************************************************************/
//static INT8U  ConstSpeedValue = 10;    // ��ʼ�ٶȣ���λʱ���ڵ����������
//static INT16U SamplingPeriod  = 5;     //�������� ��λms   
//static INT8U  MIN_UpdateUnit  = 5;     //��С���·ֶ�(ÿ�μӼ��ٶ������Ϊ�ֶ�)
//static INT16U MaxFrequency    = 200;   //���Ƶ�� ��λHZ
//static INT16U uTIM_Prescler   = 720;   //��Ƶϵ��
/***************************************************************/
const uint16_t KeepMinPulse = 0;
T1_TypeDef T1ParaCfg[MAX_NUM_X];
const INT16U ConstTimPrescler[] = {72,720,7200,36000};// 1us 10us 100us 2000us

//uint16_t TimOverflowCnt = 0;           // �ñ��������жϵ�ǰʱ��ε������Ƿ�������
uint16_t PlusKeepCnt    = 0;
uint32_t PulsesNumber = 0;     // ��׽�����������

uint8_t  StypeStatus = NOKEEP_STATUS;  // �����Ƿ�Ҫ�Ӽ����ж�
uint8_t  SpeedStatus = SPD_INIT;          //�Ӽ���״̬

uint32_t FrequencyMultiplication = 1;     // ��Ƶ����
INT8U AxisChoice = 1;                  // ����ѡ��

/*****************************************************************************
*
******************************************************************************/
void Tim1_init(void) 
{
  TIM_BDTRInitTypeDef TIM1_BDTRInitStruct;
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  Speed_t pstepSpeed;  

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
//GPIO_ResetBits(GPIOA,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);

  TIM_BDTRStructInit(&TIM1_BDTRInitStruct);
  
  TIM_DeInit(TIM1); 
  pstepSpeed = &stepSped;
  TIM_TimeBaseStructure.TIM_Period = pstepSpeed->pt1_cfg_tcb[0].time_period;///(pstepSpeed->pT1CfgTcb[pstepSpeed->pcur_tcb].tim_overflow_cnt+1);
  TIM_TimeBaseStructure.TIM_Prescaler = pstepSpeed->pt1_cfg_tcb[0].tim_prescaler-1; // 10us
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 

  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;// ÿn�θ������

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  
  /* PWM1 Mode configuration: Channel1 */   
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//TIM_OCMode_PWM1; 
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // ʹ������Ƚ�״̬
  TIM_OCInitStructure.TIM_Pulse = (pstepSpeed->pt1_cfg_tcb[0].time_period)/2;   	      
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//TIM_OCPolarity_Low;//
  // ѡ�����״̬�µķǹ���״̬// ��MOE=0����TIM1����ȽϿ���״̬// Ĭ�����λ�͵�ƽ
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//TIM_OCIdleState_Set;//
  
  /* PWM1 Mode configuration: Channel1 */  
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_ForcedOC1Config(TIM1, TIM_ForcedAction_Active);
  TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);//
  /* PWM1 Mode configuration: Channel2 */   
  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_ForcedOC2Config(TIM1, TIM_ForcedAction_Active);
  TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
  /* PWM1 Mode configuration: Channel3 */   
  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_Active);
  TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
  /* PWM1 Mode configuration: Channel4 */    	      
  TIM_OC4Init(TIM1, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_ForcedOC4Config(TIM1, TIM_ForcedAction_Active);
  TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Disable);

  TIM_ARRPreloadConfig(TIM1, DISABLE);//ʹ��TIM1��ARR�ϵ�Ԥװ�ؼĴ��� DISABLE
  TIM_ClearFlag(TIM1,TIM_FLAG_Update);
  TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);	// ��ʱ�ж�ʹ��DISABLE
  TIM_Cmd(TIM1, ENABLE);		        // ��ʱ����ʼ����
  TIM_CtrlPWMOutputs(TIM1, ENABLE);             // ����������Ҫ��!!!
}

void Tim1_DesiableChannel(void) 
{
  TIM_OCInitTypeDef TIM_OCInitStructure;
  
  /* PWM1 Mode configuration: Channel1 */  
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
  /* PWM1 Mode configuration: Channel2 */   
  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
  /* PWM1 Mode configuration: Channel3 */   
  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
  /* PWM1 Mode configuration: Channel4 */    	      
  TIM_OC4Init(TIM1, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
  TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Disable);

//  TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);	// ��ʱ�ж�ʹ��
//  TIM_ClearFlag(TIM1,TIM_FLAG_Update);
}

void PerStepTimConfigInit(void)// ÿ���ٶȽ���TIME1������
{
  Speed_t pstepSpeed;  
  long steptime;//ÿ�����ߵ�ʱ��
  
  pstepSpeed = &stepSped;
  
  for(char i=0;i<pstepSpeed->v_gradient_num;i++)
  {
    // pstepSpeed->t_unit��λʱ�� pstpSpeed->UnitCn��λʱ�������߾���������������
    steptime = (long)((pstepSpeed->t_unit/(*(pstepSpeed->unit_cnt+i)))*1000000);//1us
    pstepSpeed->pt1_cfg_tcb[i].tim_prescaler = ConstTimPrescler[0];
    pstepSpeed->pt1_cfg_tcb[i].time_period   = (int)((pstepSpeed->t_unit*1000000)/pstepSpeed->unit_cnt[i]);
    if(steptime > 0xffff)
    {
      steptime = (long)((pstepSpeed->t_unit/(*(pstepSpeed->unit_cnt+i)))*100000);//10us
      pstepSpeed->pt1_cfg_tcb[i].tim_prescaler = ConstTimPrescler[1];
      pstepSpeed->pt1_cfg_tcb[i].time_period   =  (int)((pstepSpeed->t_unit*10000)/pstepSpeed->unit_cnt[i]);
      if(steptime > 0xffff)
      {
        steptime = (long)((pstepSpeed->t_unit/(*(pstepSpeed->unit_cnt+i)))*10000);//100us
        pstepSpeed->pt1_cfg_tcb[i].tim_prescaler = ConstTimPrescler[2];
        pstepSpeed->pt1_cfg_tcb[i].time_period   =  (int)((pstepSpeed->t_unit*1000)/pstepSpeed->unit_cnt[i]);
        if(steptime > 0xffff)
        {
          steptime = (long)((pstepSpeed->t_unit/(*(pstepSpeed->unit_cnt+i)))*1000);//1000us
          pstepSpeed->pt1_cfg_tcb[i].tim_prescaler = ConstTimPrescler[2];
          pstepSpeed->pt1_cfg_tcb[i].time_period   =  (int)((pstepSpeed->t_unit*500)/pstepSpeed->unit_cnt[i]);
        }
      }
    }

    pstepSpeed->pt1_cfg_tcb[i].tim_overflow_cnt = pstepSpeed->unit_cnt[i]/256; // 0 - 255
    pstepSpeed->pt1_cfg_tcb[i].tim_repetition_cnt = pstepSpeed->unit_cnt[i]%256;//����

    pstepSpeed->pcur_tcb  = 0;
  }
}

//�Ӽ��ټ��� 
INT8U IntSpeedCalculation(void)
{
  Speed_t speed = &stepSped;
  
  // Ҫ��״̬���������κδ���
  if((SPD_KEEP_ACCELERATE == SpeedStatus)||(SPD_KEEP_DOWN == SpeedStatus)||
     (SPD_KEEP_UNIFORM == SpeedStatus)||
    (SPD_KEEP_REMNANT == SpeedStatus)||(SPD_KEEP_UNIFORM == SpeedStatus))
  {
    return SpeedStatus;
  }
  
  // û������ �ر����
  if(!PulsesNumber)
  {
    return SPD_STOP;
  }
  
  /*
  ֱ������������
  1��С�������ٶ�������
  2����δ����ӡ�����״̬���Ѿ������ʣ������ speed->pcur_tcb == 0
  */
  if((!speed->pcur_tcb)&&(PulsesNumber <= speed->total_cnt[0]))
  {
    return SPD_REMNANT;
  }

  // ��������:��ǰ��׽���������� >= ��һ���ٶȵ�����������+�Ѿ����ĵ���������
  if(speed->pcur_tcb < speed->v_gradient_num-1)
  {
    if(PulsesNumber >= speed->total_cnt[speed->pcur_tcb+1]) // ����
    {
      return SPD_ACCELERATE;
    }
    
    //��������:��һ���ٶȵ�����������+�Ѿ����ĵ���������>��ǰ��׽���������� > �Ѿ����ĵ�����������
    if(((speed->total_cnt[speed->pcur_tcb]) < PulsesNumber) && (PulsesNumber < (speed->total_cnt[speed->pcur_tcb+1])))
    {
      return SPD_UNIFORM;
    }
  }
  else
  {
    if(PulsesNumber >= speed->total_cnt[speed->pcur_tcb])// ���ٵ��˼��ޣ�ֻ�ܱ�������
    {
      return SPD_UNIFORM;
    }
    return SPD_SLOW_DOWN;
  }

  // �����������������
  
  return SPD_SLOW_DOWN;
}

void _stop_timmer(Speed_t pstepSpeed)
{
  //__disable_irq();
  MotorRunningDisable();
//  Tim1_DesiableChannel();
  TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);	// ��ʱ�ж�ʹ��
  TIM_ClearFlag(TIM1,TIM_FLAG_Update);
//  TIM_CtrlPWMOutputs(TIM1, DISABLE);             // ����������Ҫ��!!!
  PulsesNumber = 0;
  SpeedStatus = SPD_INIT;
  pstepSpeed->pcur_tcb = 0;
  //__enable_irq();
  return ;
}

void StopTimmer(void)
{
  Speed_t pstepSpeed;  
  
  pstepSpeed = &stepSped;
  
  MotorRunningDisable();
  TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);	// ��ʱ�ж�ʹ��
  TIM_ClearFlag(TIM1,TIM_FLAG_Update);
  PulsesNumber = 0;
  SpeedStatus = SPD_INIT;
  pstepSpeed->pcur_tcb = 0;
  return ;
}

void _spd_config(int tim_period,int tim_prescaler,int repetition_cnt,char axis_choice)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure; 
  
  TIM_TimeBaseStructure.TIM_Period = tim_period;
  TIM_TimeBaseStructure.TIM_Prescaler = tim_prescaler-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
  TIM_TimeBaseStructure.TIM_RepetitionCounter = repetition_cnt-1;//repetition_cnt-1
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  
  /* PWM1 Mode configuration: Channel1 */   
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//; 
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // ʹ������Ƚ�״̬
  TIM_OCInitStructure.TIM_Pulse = tim_period/2;   	      
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//TIM_OCPolarity_Low;//
  // ѡ�����״̬�µķǹ���״̬// ��MOE=0����TIM1����ȽϿ���״̬// Ĭ�����λ�͵�ƽ
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//TIM_OCIdleState_Set;//
  
  switch(axis_choice)
  {
  case 1:/* PWM1 Mode configuration: Channel1 */  
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���TIM_OCPreload_Enable
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
    led_toggle(LED_1);

    /* PWM1 Mode configuration: Channel2 */   
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC2Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel3 */   
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel4 */    	      
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC4Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Disable);
    break;
  case 2:/* PWM1 Mode configuration: Channel2 */   
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
    led_toggle(LED_2);
    
    /* PWM1 Mode configuration: Channel1 */  
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC1Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel3 */   
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel4 */    	      
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC4Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Disable);
    break;
  case 3:/* PWM1 Mode configuration: Channel3 */   
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
    led_toggle(LED_3);
    
    /* PWM1 Mode configuration: Channel1 */  
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC1Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel2 */   
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC2Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel4 */    	      
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC4Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Disable);
    break;
  case 4:/* PWM1 Mode configuration: Channel4 */    	      
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Enable);
    led_toggle(LED_4);
    
    /* PWM1 Mode configuration: Channel1 */  
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC1Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel2 */   
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC2Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel3 */   
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
    break;
  default:
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���TIM_OCPreload_Enable
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
    led_toggle(LED_1);
    
    /* PWM1 Mode configuration: Channel2 */   
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC2Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel3 */   
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC3Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);
    /* PWM1 Mode configuration: Channel4 */    	      
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);// ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ForcedOC4Config(TIM1, TIM_ForcedAction_Active);
    TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Disable);
    break;
  }
  
  TIM_ClearFlag(TIM1,TIM_FLAG_Update);
  TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);	// ��ʱ�ж�ʹ��
//  TIM_CtrlPWMOutputs(TIM1, ENABLE);
  return ;
}

uint8_t Tim1_Configuration(uint8_t speed_status) 
{
  Speed_t pstepSpeed;  
  static int isPlusKeepCnt = 0;
  static int isPlus256Cnt  = 0;
  
  pstepSpeed = &stepSped;

  switch(speed_status)
  {
  case SPD_STOP:
    _stop_timmer(pstepSpeed);
    led_off(LED_1);
    led_off(LED_2);
    led_off(LED_3);
    led_off(LED_4);
    break;
  case SPD_REMNANT:
    isPlus256Cnt  = pstepSpeed->pt1_cfg_tcb[0].tim_overflow_cnt;
    isPlusKeepCnt = pstepSpeed->plus_keep_cnt[0];
    if((isPlus256Cnt || isPlusKeepCnt)&&(PulsesNumber >= 256 ))
    {
      SpeedStatus = SPD_KEEP_REMNANT;
    }
    else
    {
      _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                  pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                  PulsesNumber,AxisChoice);
      PulsesNumber = 0;    
      SpeedStatus = SPD_STOP;
      break;
    }
  case SPD_KEEP_REMNANT:  
    if(pstepSpeed->plus_keep_cnt[0])// �����Ҫ�ڱ��ٶ��±���
    {
      if(pstepSpeed->pt1_cfg_tcb[0].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          if(PulsesNumber > 256)
          {
            _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                        pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                        256,AxisChoice);
            PulsesNumber -= 256;
            isPlus256Cnt --;            
          }
          else
          {
            goto loop1;
          }
        }
        else
        {
        loop1:
          if(isPlusKeepCnt)
          {
            _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                        pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                        pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt,AxisChoice);
            PulsesNumber -= pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt;
            isPlusKeepCnt --;
            isPlus256Cnt = pstepSpeed->pt1_cfg_tcb[0].tim_overflow_cnt;
            break;
          }
          else
          {
            _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                        pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                        PulsesNumber,AxisChoice);
            PulsesNumber = 0;
             SpeedStatus = SPD_STOP;
             break;
          }
        }
      }
      else //һ�ο��Ա�ʾ��
      {
        if(PulsesNumber > pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                      pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt;
          
          isPlusKeepCnt --;
          if(!isPlusKeepCnt)
          {
            SpeedStatus = SPD_REMNANT;
            break;
          }
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                      pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                      PulsesNumber,AxisChoice);
          PulsesNumber = 0;
          SpeedStatus = SPD_STOP;
        }
      }
    }
    else
    {
      if(pstepSpeed->pt1_cfg_tcb[0].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          if(PulsesNumber > 256)
          {
            _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                        pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                        256,AxisChoice);
            PulsesNumber -= 256;
            isPlus256Cnt --;            
          }
          else
          {
            goto loop2;
          }
        }
        else
        {
        loop2:
          if(PulsesNumber > pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt)
          {
            _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                        pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                        pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt,AxisChoice);
            PulsesNumber -= pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt;
            SpeedStatus = SPD_REMNANT; 
            break;
          }
          else
          {
            _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                        pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                        PulsesNumber,AxisChoice);
            PulsesNumber = 0;
            SpeedStatus = SPD_STOP; 
            break;
          }
        }
      }
      else
      {
        if(PulsesNumber > pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                      pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[0].tim_repetition_cnt;
          SpeedStatus = SPD_REMNANT;
          break;          
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[0].time_period,
                      pstepSpeed->pt1_cfg_tcb[0].tim_prescaler,
                      PulsesNumber,AxisChoice);
          PulsesNumber = 0;
          SpeedStatus = SPD_STOP;
          break;  
        }
      }
    }
    break;
  case SPD_ACCELERATE:
    pstepSpeed->pcur_tcb ++;
    isPlus256Cnt  = pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt;
    isPlusKeepCnt = pstepSpeed->plus_keep_cnt[pstepSpeed->pcur_tcb];
    if(isPlus256Cnt || isPlusKeepCnt)
    {
      SpeedStatus = SPD_KEEP_ACCELERATE;
    }
    else
    {
      _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                  pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                  pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
      PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
      SpeedStatus = SPD_ACCELERATE;
      break;
    }
  case SPD_KEEP_ACCELERATE:
    if(pstepSpeed->plus_keep_cnt[pstepSpeed->pcur_tcb])// �����Ҫ�ڱ��ٶ��±���
    {
      if(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      256,AxisChoice);
          PulsesNumber -= 256;
          isPlus256Cnt --;
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
          
          if(isPlusKeepCnt)
          {
            isPlusKeepCnt --;
            isPlus256Cnt = pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt;
            break;
          }
          else
          {
             SpeedStatus = SPD_ACCELERATE;
             break;
          }
        }
      }
      else //һ�ο��Ա�ʾ��
      {
        _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
        PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
        
        isPlusKeepCnt --;
        if(!isPlusKeepCnt)
        {
          SpeedStatus = SPD_ACCELERATE;
          break;
        }
      }
    }
    else
    {
      if(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      256,AxisChoice);
          PulsesNumber -= 256;
          isPlus256Cnt --;
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
          SpeedStatus = SPD_ACCELERATE;
          break;
        }
      }
      else
      {
        _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
        PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
        SpeedStatus = SPD_ACCELERATE;
        break;
      }
    }
    break;
  case SPD_SLOW_DOWN:
    pstepSpeed->pcur_tcb --;
    isPlus256Cnt  = pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt;
    isPlusKeepCnt = pstepSpeed->plus_keep_cnt[pstepSpeed->pcur_tcb];
    if(isPlus256Cnt || isPlusKeepCnt)
    {
      SpeedStatus = SPD_KEEP_DOWN;
    }
    else
    {
      _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                  pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                  pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
      PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
      SpeedStatus = SPD_SLOW_DOWN;
      break;
    }
  case SPD_KEEP_DOWN:
    if(pstepSpeed->plus_keep_cnt[pstepSpeed->pcur_tcb])// �����Ҫ�ڱ��ٶ��±���
    {
      if(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      256,AxisChoice);
          PulsesNumber -= 256;
          isPlus256Cnt --;
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
          
          if(isPlusKeepCnt)
          {
            isPlusKeepCnt --;
            isPlus256Cnt = pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt;
            break;
          }
          else
          {
             SpeedStatus = SPD_SLOW_DOWN;
             break;
          }
        }
      }
      else //һ�ο��Ա�ʾ��
      {
        _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
        PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
        
        isPlusKeepCnt --;
        if(!isPlusKeepCnt)
        {
          SpeedStatus = SPD_SLOW_DOWN;
          break;
        }
      }
    }
    else
    {
      if(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      256,AxisChoice);
          PulsesNumber -= 256;
          isPlus256Cnt --;
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
          SpeedStatus = SPD_SLOW_DOWN;
          break;
        }
      }
      else
      {
        _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
        PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
        SpeedStatus = SPD_SLOW_DOWN;
        break;
      }
    }
    break;
  case SPD_UNIFORM:
    pstepSpeed->pcur_tcb = pstepSpeed->pcur_tcb;
    isPlus256Cnt  = pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt;
    isPlusKeepCnt = pstepSpeed->plus_keep_cnt[pstepSpeed->pcur_tcb];
    if(isPlus256Cnt || isPlusKeepCnt)
    {
      SpeedStatus = SPD_KEEP_UNIFORM;
    }
    else
    {
      _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                  pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                  pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
      PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
      SpeedStatus = SPD_UNIFORM;
      break;
    }
  case SPD_KEEP_UNIFORM:
    if(pstepSpeed->plus_keep_cnt[pstepSpeed->pcur_tcb])// �����Ҫ�ڱ��ٶ��±���
    {
      if(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      256,AxisChoice);
          PulsesNumber -= 256;
          isPlus256Cnt --;
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
          
          if(isPlusKeepCnt)
          {
            isPlusKeepCnt --;
            isPlus256Cnt = pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt;
            break;
          }
          else
          {
             SpeedStatus = SPD_UNIFORM;
             break;
          }
        }
      }
      else //һ�ο��Ա�ʾ��
      {
        _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
        PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
        
        isPlusKeepCnt --;
        if(!isPlusKeepCnt)
        {
          SpeedStatus = SPD_UNIFORM;
          break;
        }
      }
    }
    else
    {
      if(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_overflow_cnt)// ���һ�α�ʾ����
      {
        if(isPlus256Cnt)
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      256,AxisChoice);
          PulsesNumber -= 256;
          isPlus256Cnt --;
        }
        else
        {
          _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                      pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
          PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
          SpeedStatus = SPD_UNIFORM;
          break;
        }
      }
      else
      {
        _spd_config(pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].time_period,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_prescaler,
                    pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt,AxisChoice);
        PulsesNumber -= pstepSpeed->pt1_cfg_tcb[pstepSpeed->pcur_tcb].tim_repetition_cnt;
        SpeedStatus = SPD_UNIFORM;
        break;
      }
    }
    break;
  default:
    break;
  }
  return 0;
}

// ����ѡ��ʧ�ܵ�ǰ��
void AxisChoiseSwitchDisable(void)
{
  PWMALL_OFF();
}

/*ʧ�ܵ����ת*/
void MotorRunningDisable(void)
{ 
  AXIS_UNCHOICE(1);
  AXIS_UNCHOICE(2);
  AXIS_UNCHOICE(3);
  AXIS_UNCHOICE(4);
//  GPIO_SetBits(GPIOB, GPIO_Pin_14);
//  TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);	// ��ʱ�ж�ʹ��
}
/*ʹ�ܵ����ת*/
void MotorRunningEnable(void)
{
  GPIO_ResetBits(GPIOB, GPIO_Pin_14);
//  TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);	// ��ʱ�ж�ʹ��
}

/*
 * ��Ҫ��ɣ������ֹͣ����������һ�ν���ʱ����Ҫ��ȡ
 * �����ת����ѡ����˶��ἰ��Ƶ����Ȼ����ݱ�Ƶ��
 * �������塣
 * ��һ�����жϱ�ʾ������������Ҫ�ٴζ�ȡһ�ε����ת����
 * ѡ����˶��ἰ��Ƶ��
*/
void IntFrequencyMAxisCapture(void)
{ 
  uint8_t tmp_mult = 0;
//  GPIO_ResetBits(GPIOB, GPIO_Pin_14);
  if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0))
  {// ��1
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1))
    {// ��ȡ�����ת����
      GPIO_SetBits(GPIOB, GPIO_Pin_13);
    }// ���÷���
    else
    {
      GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    }
    AxisChoice = 1;
  }
  else if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10))
  {// ��2
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1))
    {// ��ȡ�����ת����
      GPIO_SetBits(GPIOB, GPIO_Pin_4);//GPIO_SetBits
    }
    else
    {
      GPIO_ResetBits(GPIOB, GPIO_Pin_4);//GPIO_ResetBits
    }
    AxisChoice = 2;
  }
  else if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11))
  {// ��3
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1))
    {// ��ȡ�����ת����
      GPIO_SetBits(GPIOB, GPIO_Pin_5); 
    }
    else
    {
      GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    }
    AxisChoice = 3;
  }
  else if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12))
  {// ��4
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1))
    { // ��ȡ�����ת����
      GPIO_SetBits(GPIOB, GPIO_Pin_6);
    }
    else
    {
      GPIO_ResetBits(GPIOB, GPIO_Pin_6);
    }
    AxisChoice = 4;
  }
  else
  {
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1))
    {// ��ȡ�����ת����
      GPIO_SetBits(GPIOB, GPIO_Pin_13);
    }// ���÷���
    else
    {
      GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    }
    AxisChoice = 1;
  }

  tmp_mult = 0;
  tmp_mult = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15);
  tmp_mult <<= 1;
  tmp_mult |= GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14);
  tmp_mult <<= 1;
  tmp_mult |= GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
  switch(tmp_mult)
  {
  case 0:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[7];
    break;
  case 1: 
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[8];
    break;
  case 2:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[9];
    break;
  case 3:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[10];
    break;
  case 4:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[11];
    break;
  case 5:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[12];
    break;
  case 6:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[13];
    break;
  case 7:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[14];
    break;
  default:
    FrequencyMultiplication = NOR_FlashWriteBuf[1]*NOR_FlashWriteBuf[8];
    break;
  }
//  TIM_CtrlPWMOutputs(TIM1, ENABLE);             // ����������Ҫ��!!!
}

// ��ȡ��������
INT32U GetCapturePulsesValue(void)
{
  return PulsesNumber;
}