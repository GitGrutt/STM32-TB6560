#include "stepper_motor.h"
#include "st_flashdriver.h"

extern uint32_t NOR_FlashWriteBuf[FLASH_MSG_LEN];

//#define MIN_PULSE_NUM               50 // ��ĳ���ٶ��±��ֵ���С������
//�Ӽ��ٷ���2��֮һ�����3��֮һ����  
//#define SPEEDPRINCIPLE              1//3  
//������ת�٣���λת/min  
//#define MOTOMAXSPEED                1500      
//���ÿת��Ӧ�Ĵ�������,��λ��mm  
//#define MOTOROUNDLENGTH             4  
//��������ٶȣ����ٶȳ�����ֵ��Ե������˺�����λת/min  
//#define MOTOMAXSTART                200  
//������ϸ����  
//#define DRIVERSFRACTION             1  
//���ϸ����  
//#define MOTOFRACTION                200  
//���ݳ��ٶȣ�ĩ�ٶȣ��Լ�ʱ�䣬�������·��  
#define ACCELERATESPACE(V0,Vt,t)    (((V0) + (Vt)) * (t) / 2)  
//���ݼ���·�̣����ٶȣ��Լ�ʱ�䣬����ĩ�ٶ�  
#define LASTVELOCITY(S,V0,t)        (2 * (S) / (t) - (V0))  
//���ݵ��ת�٣�ת/min�������������ٶȣ�step/s��  
#define STEPSPEED(RV)               ((RV) * NOR_FlashWriteBuf[2] * NOR_FlashWriteBuf[1] / 60)//((RV) * MOTOFRACTION * DRIVERSFRACTION / 60)  
//���ݳ��ȼ����������ߵĲ�����S�ĵ�λΪmm  
#define TOTALSTEP(S)                S * NOR_FlashWriteBuf[2] * NOR_FlashWriteBuf[1] / NOR_FlashWriteBuf[16] //S * MOTOFRACTION * DRIVERSFRACTION / MOTOROUNDLENGTH   

int sped_tab[MAX_NUM_X];      // �ٶ��б�
int cur_per_dis[MAX_NUM_X];   // ��ǰ��λʱ�����ĵ�������
int cur_total_dis[MAX_NUM_X]; // ��ǰ�ܹ����ĵ�������
int cur_keep_cnt[MAX_NUM_X];  // ��ǰ�ٶ��±��ִ���������Ϊ0
extern T1_TypeDef T1ParaCfg[MAX_NUM_X];

strSpeed                            stepSped;

//�������ƣ� CalculateSpeedTab  
//�������ܣ�����Ӽ��ٱ�  
//����������Speed �ٶȽṹ�� 
//����������ٶȽṹ��  
static Speed_t CalculateSpeedTab(Speed_t Speed)
{  
  int i;                          
  double aa;                    //�Ӽ���  
  double per_s;
  int DeltaV;                   //�ٶȱ仯��  
  int tmpValue = 0;

  // ����Ӽ��ٶ�
  aa = (double)((Speed->Vt - Speed->V0) / 2)      //���ٶȱ仯�յ�ʱ���ٶ�ֵ                           a|  /|\ ���ΪDeltaV              
    * 2                                         //֪��ֱ����������������������͸߶�ʱ�����*2        |  / | \���ΪDeltaV  
      / (Speed->time / 2)                         //���Եױ�                                            | /  |  \���ΪDeltaV              
        / (Speed->time / 2);                        //�ٳ��Եױߣ��õ�б�ʣ����Ӽ���                      |/___|___\__time     
  
  /*��ʼ�ٶȼ��� �õ��ٶ��б� */
  for(i = 0; i < ((Speed->time / 2) + 1); i++)  
  {  
    DeltaV = (int)((aa * i * i) / 2);                              //V = V0 + a * t / 2;  a = aa * t;  
    *(Speed->speed_tab + i) = Speed->V0 + DeltaV;                   //��ǰ����ٶ� 
    *(Speed->speed_tab + Speed->time - i) = Speed->Vt - DeltaV;     //�ԳƵ���ٶ�
  }
  
  Speed->pulse_keep_min = NOR_FlashWriteBuf[0];//MIN_PULSE_NUM;
  Speed->v_gradient_num = 0;
  
  /*�õ���λʱ���� �����б�*/
  for(i = 0; i < (Speed->time + 1); i++) 
  { 
    if(0 == i)
    {
      per_s = (double)((ACCELERATESPACE(Speed->V0,Speed->V0,Speed->t_unit)));
      if((int)per_s)
      {
        *(Speed->unit_cnt+Speed->v_gradient_num) = (int)per_s;//(int)TOTALSTEP(per_s);
        Speed->v_gradient_num ++;
      }
    }
    else
    {
      per_s = (double)((ACCELERATESPACE(*(Speed->speed_tab + i-1),*(Speed->speed_tab + i),Speed->t_unit)));
      if((int)per_s)
      {
        *(Speed->unit_cnt+Speed->v_gradient_num) = (int)per_s;//(int)TOTALSTEP(per_s);
        if(*(Speed->unit_cnt+Speed->v_gradient_num-1) != *(Speed->unit_cnt+Speed->v_gradient_num))
        {
          Speed->v_gradient_num ++;
        }
      }
    }
  }
  
  for(i=0;i<Speed->v_gradient_num;i++)
  {
    if(*(Speed->unit_cnt+i) < Speed->pulse_keep_min)
    {
      if(Speed->pulse_keep_min%(*(Speed->unit_cnt+i)))//�Ƿ�������
      {
        *(Speed->plus_keep_cnt+i) = Speed->pulse_keep_min/(*(Speed->unit_cnt+i))+1;
      }
      else
      {
        *(Speed->plus_keep_cnt+i) = Speed->pulse_keep_min/(*(Speed->unit_cnt+i));
      }
    }
    else
    {
      *(Speed->plus_keep_cnt+i) = 0;
    }
    
    if(*(Speed->plus_keep_cnt+i))
    {
      tmpValue += (*(Speed->plus_keep_cnt+i))*(*(Speed->unit_cnt+i));
    }
    else
    {
      tmpValue += *(Speed->unit_cnt+i);
    }
    
    *(Speed->total_cnt+i) = tmpValue;
  }
  
  return Speed;
}

//�������ƣ�TestSpeed  
//�������ܣ��ٶȱ���㺯��  
//����������V0      ���ٶȣ���λ��ת/min  
//          Vt      ĩ�ٶȣ���λ��ת/min  
//          total_time     ������ʱ��
//          unit_time      ��λʱ�� 
//�������أ���
void TestSpeed(int V0, int Vt,double total_time,double unit_time)  
{  
  Speed_t Speed = &stepSped;  
  Speed->V0 = STEPSPEED(V0);//STEPSPEED(V0);    // ����/ 
  Speed->Vt = STEPSPEED(Vt);//STEPSPEED(Vt);    // ĩ�� / 
//  Speed->S = S;//TOTALSTEP(S);       // ·��  
//  Speed->t = time;                   // ����ʱ����Ϊ0.2��
  if(total_time/unit_time > MAX_NUM_X-1)
  {
    Speed->time = MAX_NUM_X-1;
  }
  else
  {
    Speed->time = (int)(total_time/unit_time);                 // ���ٴ��� 
  }
  
  Speed->t_unit = unit_time;              // ��λʱ��
  Speed->pcur_tcb = 0;               // ��ǰ���������λ��
  
  Speed->unit_cnt = cur_per_dis;   // ������������
  Speed->total_cnt = cur_total_dis;   // ��ǰ�ܾ���
  Speed->speed_tab = sped_tab;         // ���ݼ��ٴ����������ڴ棬��1���Է��õ�ʱ�򳬽�  
  Speed->pt1_cfg_tcb = T1ParaCfg;      // ÿ�����ʱ������
  Speed->plus_keep_cnt  = cur_keep_cnt;
  CalculateSpeedTab(Speed);          // ��ʼ����
}