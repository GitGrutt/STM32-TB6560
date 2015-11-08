/**
  ******************************************************************************
  * @file    main.c
  * @author  
  * @version V1.0.0
  * @date    07/13/2014
  * @brief 
  --- led
  *LED1---------PB5
  *LED2---------PB6
  *LED3---------PB7
  *LED4---------PB8
  --- time1
  *TIM1 CH3  ------PA10
  *TIM1 CH3N ------ PB15 
  --- dma addr
  *TIM1_CCR3_Address    0x40012C3C
  --- External Interrupt
  *
  ******************************************************************************
**/

#include "stm32f10x.h"
#include "sys.h"
#include "stepper_motor.h"
#include "config_model.h"
#include  "usart1_reader.h" 
#include "st_flashdriver.h"
#include "clpd_clk.h"

#include <string.h>

#define STEP0      0
#define STEP1      1

#define RCC_APB2Periph_WATCHDOG     RCC_APB2Periph_GPIOB
#define GPIOWATCHDOG                GPIOB
#define WATCHDOGPIN                 GPIO_Pin_15

//#define DEBUG

uint32_t NOR_FlashWriteBuf[FLASH_MSG_LEN];

INT8U StepxStatus = STEP0;

INT16U rand_vlaue;
extern volatile INT32U PulsesNumber;
extern uint8_t Tim1OverFlag;

void ConfigProcess(void);
void SoftReset(void) ;

/*
SP706 �� Exar��ԭ Sipex����˾�Ƴ��ĵ͹��ġ��߿ɿ����ͼ۸�� MCU��λ���оƬ��
��������ؼ����ԣ� 
z  ��Ϊ 4 �����ͺţ�SP706P��SP706R��SP706S��SP706T 
z  ��λ�����PΪ�ߵ�ƽ��Ч��R/S/T Ϊ�͵�ƽ��Ч 
z  ���ܵĵ͵�ѹ��أ�P/RΪ 2.63V��SΪ 2.93V��TΪ 3.08V 
z  ��λ�����ȣ�200ms���ֵ�� 
z  �����Ŀ��Ź���ʱ����1.6 �볬ʱ���ֵ�� 
z  ȥ�� TTL/CMOS �ֶ���λ���루/MR �ܽţ� 
*/
void WDG_SP706_GPIO_Configration(void)
{
#if DEBUG
  GPIO_InitTypeDef  gpio_init;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_WATCHDOG, ENABLE);
  
  //GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE); 
  gpio_init.GPIO_Pin   = WATCHDOGPIN;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOWATCHDOG, &gpio_init);	
#endif
}

void WDG_SP706_Enable(void)
{
#if DEBUG
  GPIO_InitTypeDef  gpio_init;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_WATCHDOG, ENABLE);
  
  gpio_init.GPIO_Pin   = WATCHDOGPIN;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOWATCHDOG, &gpio_init);
#endif
}

void WDG_SP706_Feed(void)
{
#if DEBUG  
  static u8 reg_val = 0;
  reg_val = GPIO_ReadOutputDataBit(GPIOWATCHDOG , WATCHDOGPIN);
//  reg_val = ~reg_val;
  if(1 == reg_val)
  {
    GPIO_ResetBits(GPIOWATCHDOG, WATCHDOGPIN);
    delayms(10);	
    return;
  }
  GPIO_SetBits(GPIOWATCHDOG, WATCHDOGPIN);   
  delayms(10);	
#endif
}

void Wdt_Init(void)
{
#if DEBUG
  // Enable write access to IWDG_PR and IWDG_RLR registers 
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //IWDG->KR = 0x5555
  // IWDG counter clock: 40KHz(LSI) / 64 = 0.625 KHz 
  IWDG_SetPrescaler(IWDG_Prescaler_16);   
  // Set counter reload value to 1250 
  IWDG_SetReload(0xfff);           //IWDG->RLR =0xFFF
  //Red IWDG counter
  IWDG_ReloadCounter();   //IWDG->KR = 0xAAAA
  // Enable IWDG (the LSI oscillator will be enabled by hardware) 
  IWDG_Enable();   //IWDG->KR = 0xCCCC
#endif
}

//ι�� 
void Kick_Dog(void)
{
#if DEBUG
  //Reload IWDG counter 
  IWDG_ReloadCounter();   //IWDG->KR = 0xAAAA   
//  WDG_SP706_Enable();
  WDG_SP706_Feed();
#endif
} 

static void buzzerOn(void)
{
  GPIO_SetBits(GPIOA, GPIO_Pin_12);
}

static void buzzerOff(void)
{
  GPIO_ResetBits(GPIOA, GPIO_Pin_12);
}

//******************************************************************************
// ������
//******************************************************************************
void main(void)
{
  RCC_Configuration();    
  SysTick_Config1();
  Tim4_init();
  ModeInquiryInit();
  Wdt_Init();
  WDG_SP706_GPIO_Configration();
  WDG_SP706_Enable();
  //TIM_BDTRStructInit(&TIM1_BDTRInitStruct)
  delayms(100);			// ��ʱ���ȴ���ѹ�ȶ�
  Kick_Dog();
  
  if(GetModeInquiry())
  {
    delayms(5);
    if(GetModeInquiry())
    {
      delayms(5);
      if(GetModeInquiry())
      {
        delayms(5);
        if(GetModeInquiry())
        {
          buzzerOn();
          delayms(20);
          Kick_Dog();
          
          buzzerOff();
          delayms(100);
          Kick_Dog();
          
          buzzerOn();
          delayms(50);
          Kick_Dog();
          
          buzzerOff();
          delayms(50);
          Kick_Dog();
          
          delayms(100);
          Kick_Dog();
          
          buzzerOn();
          delayms(20);
          Kick_Dog();
          
          delayms(100);
          Kick_Dog();
          
          delayms(100);
          Kick_Dog();

          buzzerOff();
          delayms(20);
          Kick_Dog();
          
          ConfigProcess();// ��������ģʽ
        }
      }
    }
  }

  GPIO_Configuration();
  buzzerOff();
  GPIO_ResetBits(GPIOA,GPIO_Pin_3);
  delayms(30);
  GPIO_ResetBits(GPIOA,GPIO_Pin_4);
  delayms(30);
  GPIO_ResetBits(GPIOA,GPIO_Pin_5);
  delayms(30);
  GPIO_ResetBits(GPIOA,GPIO_Pin_6);
  delayms(30);
  GPIO_SetBits(GPIOA,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_6|GPIO_Pin_5);
  FlashMsgInit();
  ReadMsgFromFlash(NOR_FlashWriteBuf,FLASH_MSG_LEN,FLASE_BASE_ADDR);
  if(FLAG_VALUE != NOR_FlashWriteBuf[FLASH_MSG_LEN-1])
  {
    buzzerOn();
    delayms(10);
    Kick_Dog();
    buzzerOff();
    delayms(300);
    SoftReset();
  }
  //�����ٶ�Vmin  ����ٶ�Vmax     ����ʱ��  ��  ˢ������  ��Ϊ�����Ϳ���������� 
  TestSpeed(NOR_FlashWriteBuf[3], 
            NOR_FlashWriteBuf[4], 
            (double)(NOR_FlashWriteBuf[5]/10000.0), 
            (double)(NOR_FlashWriteBuf[6]/10000.0)); 
  //(int V0, int Vt, int S, dobule time,int cnt)
  PerStepTimConfigInit(); // ÿ���ٶȽ���TIME1������
  Tim1_init();
  NVIC_Configuration();
  while(1)
  {
//    for(uint8_t i=0;i<200;i++)
//    {
//      EXTI_GenerateSWInterrupt(EXTI_Line0);
//      delayms(1);
//    }
    Kick_Dog();
    delayms(100);
  }
}
uint32_t bcd_value;
uint32_t FlashWriteBuf[FLASH_MSG_LEN];
void ConfigProcess(void)
{
  
  uint32_t tmp_value;
  uint16_t cmd16 = 0;
  uint16_t dcmd16 = 0;
  uint8_t  pdata[30];
  uint8_t  tx_data[30];
  uint8_t  plen,tx_len;
  
  Usart1ModeInit();
  Usart1RxIntEnable();

  FlashMsgInit();
  ReadMsgFromFlash(FlashWriteBuf,FLASH_MSG_LEN,FLASE_BASE_ADDR);
  
  while(1)
  {
    Kick_Dog();
    if(!Usart1EventDelqueue((const uint8_t*)&FlashWriteBuf[LOCAL_ADDR],(uint8_t*)&cmd16,pdata,&plen))
    {
      switch(cmd16)
      {
      case CMD_REBOOT:
        cmd16 = REP_REBOOT;
        ReFarctory();
        Usart1SendMsgDataPackage((const uint8_t*)&cmd16,(const uint8_t*)&FlashWriteBuf[LOCAL_ADDR],
                                 pdata,plen,tx_data,&tx_len);
        Usart1Tx(tx_data,tx_len);  
        delayms(100);
        SoftReset();
        break;
      case CMD_READMSG://REP_READMSG:
        dcmd16 = *((uint16_t*)pdata);
        switch(dcmd16)
        {
        case DCMD_MIN_PULSE:// ��ĳ���ٶ��±��ֵ���С������
          *(pdata+5) = (uint8_t)FlashWriteBuf[0];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[0]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[0]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[0]>>24);
          plen = 6;
          break;
        case DCMD_DRIVER:// ������ϸ����  
          *(pdata+5) = (uint8_t)FlashWriteBuf[1];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[1]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[1]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[1]>>24);
          plen = 6;
          break;
        case DCMD_MOTOFRACTION:// ���ϸ����  
          *(pdata+5) = (uint8_t)FlashWriteBuf[2];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[2]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[2]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[2]>>24);
          plen = 6;
          break;
        case DCMD_MOTOMAXSTART:// ��������ٶȣ����ٶȳ�����ֵ��Ե������˺�����λת/min 
          *(pdata+5) = (uint8_t)FlashWriteBuf[3];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[3]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[3]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[3]>>24);
          plen = 6;
          break;
        case DCMD_MOTOMAXSPEED:// ������ת�٣���λת/min (���2400ת/min)
          *(pdata+5) = (uint8_t)FlashWriteBuf[4];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[4]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[4]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[4]>>24);
          plen = 6;
          break;
        case DCMD_ACCELE_TIM:// ������ʱ��(MS)Acceleration time �ڸ�ʱ���ڼ�������Ƴ���������
          *(pdata+5) = (uint8_t)FlashWriteBuf[5];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[5]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[5]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[5]>>24);
          plen = 6;
          break;
        case DCMD_ACCELE_PERTIM	:// ���ٵ�λʱ�䣨MS��(���ݵ�λʱ���������ٶ��ݶ�) ���25MS
          *(pdata+5) = (uint8_t)FlashWriteBuf[6];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[6]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[6]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[6]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_0:// ÿ����Ƶֵ���ã���8���� ��1
          *(pdata+5) = (uint8_t)FlashWriteBuf[7];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[7]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[7]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[7]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_1:// ÿ����Ƶֵ���ã���8���� ��2
          *(pdata+5) = (uint8_t)FlashWriteBuf[8];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[8]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[8]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[8]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_2:// ÿ����Ƶֵ���ã���8���� ��3
          *(pdata+5) = (uint8_t)FlashWriteBuf[9];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[9]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[9]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[9]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_3:// ÿ����Ƶֵ���ã���8���� ��4
          *(pdata+5) = (uint8_t)FlashWriteBuf[10];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[10]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[10]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[10]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_4:// ÿ����Ƶֵ���ã���8���� ��5
          *(pdata+5) = (uint8_t)FlashWriteBuf[11];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[11]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[11]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[11]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_5:// ÿ����Ƶֵ���ã���8���� ��6
          *(pdata+5) = (uint8_t)FlashWriteBuf[12];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[12]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[12]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[12]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_6:// ÿ����Ƶֵ���ã���8���� ��7
          *(pdata+5) = (uint8_t)FlashWriteBuf[13];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[13]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[13]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[13]>>24);
          plen = 6;
          break;
        case DCMD_FRE_MULT_7:// ÿ����Ƶֵ���ã���8���� ��8
          *(pdata+5) = (uint8_t)FlashWriteBuf[14];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[14]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[14]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[14]>>24);
          plen = 6;
          break;
        case DCMD_LOCAL_ADDR:// ������ַ����
          *(pdata+5) = (uint8_t)FlashWriteBuf[15];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[15]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[15]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[15]>>24);
          plen = 6;
          break;
        case DCMDPULSE_EQUIVA: // �������嵱��
          *(pdata+5) = (uint8_t)FlashWriteBuf[16];
          *(pdata+3) = (uint8_t)(FlashWriteBuf[16]>>8);
          *(pdata+4) = (uint8_t)(FlashWriteBuf[16]>>16);
          *(pdata+2) = (uint8_t)(FlashWriteBuf[16]>>24);
          plen = 6;
          break;
        case DCDM_VERSION:   // �汾��Ϣ
          pdata[2] = 0x00;
          pdata[3] = 0x01;
          pdata[4] = 0x00;
          pdata[5] = 0x01;
          plen = 6;
        default:
          break;
        }
        cmd16 = REP_READMSG;
        memset(tx_data,0,sizeof(tx_data));
        tmp_value = 0;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>24);
        tmp_value <<= 8;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>16);
        tmp_value <<= 8;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>8);
        tmp_value <<= 8;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]);
        tmp_value <<= 8;
        Usart1SendMsgDataPackage((const uint8_t*)&cmd16,(const uint8_t*)&tmp_value,
                                 pdata,plen,tx_data,&tx_len);
        Usart1Tx(tx_data,tx_len);
        buzzerOn();
        delayms(20);
        buzzerOff();
        delayms(20);
        break;
      case CMD_WRITEMSG://REP_WRITEMSG:
        dcmd16 = *((uint16_t*)pdata);
        tmp_value = 0;
        tmp_value |= (uint32_t)((*(pdata+2))<<24);
        tmp_value |= (uint32_t)((*(pdata+3))<<16);
        tmp_value |= (uint32_t)((*(pdata+4))<<8);
        tmp_value |= (uint32_t)(*(pdata+5));
        bcd_value = (tmp_value&0x0000000f)+10*((tmp_value>>4)&0x0000000f)
          +100*((tmp_value>>8)&0x0000000f)+1000*((tmp_value>>12)&0x0000000f)
           + 10000*((tmp_value>>16)&0x0000000f)+100000*((tmp_value>>20)&0x0000000f)
            +  1000000*((tmp_value>>24)&0x0000000f)+10000000*((tmp_value>>28)&0x0000000f);
        switch(dcmd16)
        {
        case DCMD_MIN_PULSE:// ��ĳ���ٶ��±��ֵ���С������
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[0],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_DRIVER:// ������ϸ����  
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[1],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_MOTOFRACTION:// ���ϸ����  
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[2],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_MOTOMAXSTART:// ��������ٶȣ����ٶȳ�����ֵ��Ե������˺�����λת/min 
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[3],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_MOTOMAXSPEED:// ������ת�٣���λת/min (���2400ת/min)
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[4],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_ACCELE_TIM:// ������ʱ��(MS)Acceleration time �ڸ�ʱ���ڼ�������Ƴ���������
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[5],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_ACCELE_PERTIM	:// ���ٵ�λʱ�䣨MS��(���ݵ�λʱ���������ٶ��ݶ�) ���25MS
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[6],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_0:// ÿ����Ƶֵ���ã���8���� ��1
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[7],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_1:// ÿ����Ƶֵ���ã���8���� ��2
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[8],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_2:// ÿ����Ƶֵ���ã���8���� ��3
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[9],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_3:// ÿ����Ƶֵ���ã���8���� ��4
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[10],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_4:// ÿ����Ƶֵ���ã���8���� ��5
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[11],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_5:// ÿ����Ƶֵ���ã���8���� ��6
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[12],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_6:// ÿ����Ƶֵ���ã���8���� ��7
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[13],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_FRE_MULT_7:// ÿ����Ƶֵ���ã���8���� ��8
          if(0 == bcd_value)goto loop1;
          memcpy((uint8_t*)&FlashWriteBuf[14],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMD_LOCAL_ADDR:// ������ַ����
          if((0 == bcd_value)||(bcd_value) > 65535) continue;
          memcpy((uint8_t*)&FlashWriteBuf[15],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        case DCMDPULSE_EQUIVA: // �������嵱��
          if(0 == bcd_value) continue;
          memcpy((uint8_t*)&FlashWriteBuf[16],(uint8_t*)&bcd_value,4);
          plen = 6;
          break;
        default:
          break;
        }
        cmd16 = REP_WRITEMSG;
        memset(tx_data,0,sizeof(tx_data));
        tmp_value = 0;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>24);
        tmp_value <<= 8;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>16);
        tmp_value <<= 8;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>8);
        tmp_value <<= 8;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]);
        tmp_value <<= 8;
        Usart1SendMsgDataPackage((const uint8_t*)&cmd16,(const uint8_t*)&tmp_value,
                                 pdata,plen,tx_data,&tx_len);
        Usart1Tx(tx_data,tx_len); 
        buzzerOn();
        delayms(20);
        buzzerOff();
        delayms(20);
        break;
      case CMD_WRITE_OK://REP_WRITE_OK:
        WriteMsgToFlash(FlashWriteBuf,FLASH_MSG_LEN,FLASE_BASE_ADDR);
        cmd16 = REP_WRITE_OK;
        tmp_value = 0;
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>24);
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>16);
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]>>8);
        tmp_value |= (uint8_t)(FlashWriteBuf[LOCAL_ADDR]);
        Usart1SendMsgDataPackage((const uint8_t*)&cmd16,(const uint8_t*)&tmp_value,
                                 pdata,plen,tx_data,&tx_len);
        Usart1Tx(tx_data,tx_len);
        buzzerOn();
        delayms(20);
        buzzerOff();
        delayms(20);
        Kick_Dog();
        delayms(100);
        SoftReset();
        break;
      default:
        continue;
        break;
      }   
      
    }
  loop1:
    cmd16 = 0;
    plen = 0;
    memset(pdata,0,sizeof(pdata));
    delayms(5);
  }
}

//******************************************************************************
// �����λ
//******************************************************************************
void SoftReset(void) 
{  
  __set_FAULTMASK(1);      // �ر������ж�
  NVIC_SystemReset();      // ��λ
}

//******************************************************************************
// ����˯��ģʽ
//******************************************************************************
void myPWR_EnterSleepMode(void)
{
  //PWR->CR |= CR_CWUF_Set;	/* Clear Wake-up flag */
  
  /* Set SLEEPDEEP bit of Cortex System Control Register */
  //*(__IO uint32_t *) SCB_SysCtrl |= SysCtrl_SLEEPDEEP_Set;
  /* This option is used to ensure that store operations are completed */
  
  __WFI(); 		/* Request Wait For Interrupt */
}


//******************************************************************************
#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval : None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif














