#ifndef __STEPPER_MOTOR_h
#define __STEPPER_MOTOR_h

#include "stm32f10x.h"
#include "sys.h" 

#define SPD_INIT                0       //Initialization ��ʼ��
#define SPD_STOP                1       //Stop           ֹͣ
//#define SPD_KEEP                2       //               ���ֵ�ǰ״̬
#define SPD_ACCELERATE          2       //Accelerate     ����
#define SPD_KEEP_ACCELERATE     3
#define SPD_SLOW_DOWN           4       //Slow down      ����
#define SPD_KEEP_DOWN           5
#define SPD_UNIFORM             6       //Uniform        ����
#define SPD_KEEP_UNIFORM        7       
#define SPD_REMNANT             8       //               �������崦��
#define SPD_KEEP_REMNANT        9

#define KEEP_STATUS             0x01
#define NOKEEP_STATUS           0x00

#define  MAX_NUM_X       255

#define AXIS_CHOICE(x)     PWM1_ON##x()
#define AXIS_UNCHOICE(x)   PWM1_OF##x()

#define PWM1_ON1()                 TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable)
#define PWM1_OF1()                 TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable)   
#define PWM1_ON2()                 TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable)
#define PWM1_OF2()                 TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable)
#define PWM1_ON3()                 TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable)
#define PWM1_OF3()                 TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable)   
#define PWM1_ON4()                 TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Enable)
#define PWM1_OF4()                 TIM_CCxCmd(TIM1, TIM_Channel_4, TIM_CCx_Disable)

#define PWMALL_OFF() TIM_CCxCmd(TIM1, TIM_Channel_1|TIM_Channel_2|TIM_Channel_3|TIM_Channel_4, TIM_CCx_Disable)  
#define PWMALL_ON()  TIM_CCxCmd(TIM1, TIM_Channel_1|TIM_Channel_2|TIM_Channel_3|TIM_Channel_4, TIM_CCx_Enable)

typedef struct __TIME1_ST
{
  uint16_t time_period;
  uint16_t tim_prescaler;
  uint16_t tim_overflow_cnt;  //���������ÿ���һ�μ�tim_overflow_cntһ��
  uint8_t  tim_repetition_cnt;//ÿ��tim_repetition_cnt�����һ��(���ʣ��ֵ)
}T1_TypeDef;

struct SPEED  
{  
  int         V0;                 //���ٶȣ���λ��step/s  
  int         Vt;                 //ĩ�ٶȣ���λ��step/s  
  
  int         pulse_keep_min;     // ���屣����Сֵ
  int         v_gradient_num;     // �ٶ��ݶ���
  int         time;               //���ٴ�������ֵ    ��λ���� 
  double      t_unit;             //��λʱ�� s
   
  int         pcur_tcb;           //��ǰλ��

  int         *speed_tab;         //�����ٶȱ��ٶȵ�λ��step/s  
  int         *unit_cnt;          //��λʱ�������߾���������������
  int         *total_cnt;         //��ǰ�߹�����·��
  int         *plus_keep_cnt; 

  T1_TypeDef  * pt1_cfg_tcb;       //ÿ�����ʱ������
};  
typedef struct SPEED * Speed_t;  
typedef struct SPEED   strSpeed;  
extern strSpeed stepSped;

void Tim1_init(void);
uint8_t Tim1_Configuration(uint8_t speed_status);// tim1 ��������

void PerStepTimConfigInit(void);// ÿ���ٶȽ���TIME1������
void MotorRunningDisable(void);/*��ʱ�����ʹ��*/
void MotorRunningEnable(void); /*��ʱ�����ʧ��*/

void IntFrequencyMAxisCapture(void);/*���ݱ�Ƶϵ����,��׽�ж��е���*/
INT32U GetCapturePulsesValue(void);// ��ȡ��ǰ��׽����������
void AxisChoiseSwitchDisable(void);// ����ѡ��ʧ�ܵ�ǰ��

INT16U GetCurMinPulsesNum(void);// ��ȡ�����ٶ�ʱ��С���ĵ�������
INT16U GetCurTotalPulsesNum(void);//��ȡ��ǰ��·����Ҫ�������ĳ���
INT8U IntSpeedCalculation(void);//�Ӽ��ټ��� 
void TestSpeed(int V0, int Vt,double total_time,double unit_time) ;//ͨ������������������������

void StopTimmer(void);

#endif