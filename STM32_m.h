/********************************************************************/

#define uchar		u8
#define uint		u16
#define ulong		u32

#define INT8U   	u8
#define INT8S  	 	s8
#define INT16U  	u16
#define INT16S  	s16
#define INT32U  	u32
#define INT32S  	s32
#define FP32    	float
#define FP64   	 	double

/********************************************************************/
											/*���µ�bit0ȡֵλBIT0-15*/
#define clrbit(reg,bit)   reg &= ~(bit)	/*��Ĵ�����ĳ1����λ*/
#define bitclr(reg,bit)   reg &= ~(bit)	/*��Ĵ�����ĳ1����λ*/

#define setbit(reg,bit)   reg |=  (bit)	/*���üĴ�����ĳ1����λ*/
#define bitset(reg,bit)   reg |=  (bit)	/*���üĴ�����ĳ1����λ*/
#define cplbit(reg,bit)   reg ^=  (bit)	/*�ԼĴ�����ĳ1����λȡ��*/
#define bitcpl(reg,bit)   reg ^=  (bit)	/*�ԼĴ�����ĳ1����λȡ��*/

#define  testbit(reg,bit) (reg&(bit))
#define  bittest(reg,bit) (reg&(bit))

#define regcpl(reg)		 reg = ~reg
#define regclr(reg)		 reg = 0

#define bit 	u8


//*************************************************************************
//*************************************************************************
//*************************************************************************

