#ifndef __USART1_READER_H
#define __USART1_READER_H

#include "stm32f10x.h"

#define Q_USART1_ELEMENT    30//element

#define EX_AD_DOMAIN_EN                0x68    // ����ͷ
#define EX_DATA_EX                     0x16    // ����β

/*
 * ָ�CMDL CMDH  0x0000��λ  0x0020������  0x0021 д����  0x0022 д������� 
 * �𸴣�CMDL CMDH  0x8000��λ  0x8020������  0x8021 д����  0x8022 д������� 
 * HEADER    CMDL CMDH    ADDRL ADDRH    DATA_LEN    DATAx    CRC8    END
 *  68                                                                16
*/
//����ѡ�
#define CMD_REBOOT                   0x0000  // �ָ���������
#define REP_REBOOT                   0x0080
#define CMD_READMSG                  0x2000
#define REP_READMSG                  0x2080
#define CMD_WRITEMSG                 0x2100
#define REP_WRITEMSG                 0x2180
#define CMD_WRITE_OK                 0x2200
#define REP_WRITE_OK                 0x2280

#define DCMD_MIN_PULSE               0x0001  // ��ĳ���ٶ��±��ֵ���С������
#define DCMD_DRIVER                  0x0002  // ������ϸ����  
#define DCMD_MOTOFRACTION            0x0003  // ���ϸ����  
#define DCMD_MOTOMAXSTART            0x0004  // ��������ٶȣ����ٶȳ�����ֵ��Ե������˺�����λת/min   
#define DCMD_MOTOMAXSPEED            0x0005  // ������ת�٣���λת/min (���2400ת/min)
#define DCMD_ACCELE_TIM		     0x0006  // ������ʱ��(MS)Acceleration time �ڸ�ʱ���ڼ�������Ƴ���������
#define DCMD_ACCELE_PERTIM	     0x0007  // ���ٵ�λʱ�䣨MS��(���ݵ�λʱ���������ٶ��ݶ�) ���25MS
#define DCMD_FRE_MULT_0              0x0008  // ÿ����Ƶֵ���ã���8���� ��1
#define DCMD_FRE_MULT_1		     0x0108  // ÿ����Ƶֵ���ã���8���� ��2
#define DCMD_FRE_MULT_2		     0x0208  // ÿ����Ƶֵ���ã���8���� ��3
#define DCMD_FRE_MULT_3		     0x0308  // ÿ����Ƶֵ���ã���8���� ��4
#define DCMD_FRE_MULT_4		     0x0408  // ÿ����Ƶֵ���ã���8���� ��5
#define DCMD_FRE_MULT_5		     0x0508  // ÿ����Ƶֵ���ã���8���� ��6
#define DCMD_FRE_MULT_6		     0x0608  // ÿ����Ƶֵ���ã���8���� ��7
#define DCMD_FRE_MULT_7		     0x0708  // ÿ����Ƶֵ���ã���8���� ��8
#define DCMD_LOCAL_ADDR              0x0009  // ������ַ����
#define DCMDPULSE_EQUIVA             0x000A  // ���嵱������
#define DCDM_VERSION                 0x0109  // �汾��Ϣ

void Usart1ModeInit(void);
void Usart1RxIntEnable(void);
void Usart1RxIntDisable(void);
void Usart1Tx(const u8* pbuf_msg , u8 size);

uint8_t Usart1EventEnqueue(uint8_t* pdata);// ���ݽ��� �����
uint8_t Usart1EventDelqueue(const uint8_t* addr,uint8_t* pcmd,
                            uint8_t* prdata,uint8_t* prlen);// ���� ������ ����
void Usart1SendMsgDataPackage(const uint8_t* cmd,const uint8_t* addr_domain,
                              const uint8_t* pdata,uint8_t len,
                              uint8_t* prdata,uint8_t* rlen);       // ���ݴ��

#endif