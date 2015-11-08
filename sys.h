#ifndef __SYS_H
#define __SYS_H

#include "stm32f10x.h"
#include "dzy.h"
#include "stm32_m.h"
#include "myRCC.h"  
#include "stdio.h"

#define LED_1   GPIO_Pin_3
#define LED_2   GPIO_Pin_4
#define LED_3   GPIO_Pin_5
#define LED_4   GPIO_Pin_6

void SysTick_Config1(void);// SysTick���ó�ʼ��
void RCC_Configuration(void);// ʱ�����ó�ʼ��
void NVIC_Configuration(void);// NVIC����
void GPIO_Configuration(void);// GPIO����

void ModeInquiryInit(void);
uint8_t GetModeInquiry(void);

void led_toggle(uint16_t led_pin_x);// ָʾ�� ȡ��
void led_on(uint16_t led_pin_x);// ����
void led_off(uint16_t led_pin_x);// �ص�

void func_time_tb(void);// ������ʱ����,����Ϊ TIME_TB
void delayms(INT16U cnt);// ��ʱ���򣬵�λΪ*1ms

#endif