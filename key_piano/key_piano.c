#include <reg51.h>
#include "string.h"

#define uint unsigned int
#define uchar unsigned char

sbit LSA=P2^2;
sbit LSB=P2^3;
sbit LSC=P2^4;

sbit KEY3 = P3^2;//��������K3
sbit KEY4 = P3^3;//��������K4

typedef enum {false, true} bool;
bool Record = false;        //¼�Ʊ�־λ
bool Play = false;          //���ű�־λ

uint melody[30]={0};       //����   
uint index=0;               //���������
uchar KeyValue = 20;        //����ֵ��ʾ-

unsigned int code Freqtab[] = {	//��ʱ�����ڵĳ�ʼֵ
     64021,64103,64260,64400,      //����3 4 5 6
     64524,64580,64684,64777,      //����7,����1 2 3
     64021,64103,64260,64400,      //����4 5 6 7
     65058,65110,65157,65178};     //����1 2 3 4

//��������
uint FreqTemp;
     
//������
sbit SPEAK = P1^5;
uchar led[] = {0XFE,0XFD,0XFB,0XF7,0XEF,0XDF,0XBF,0X7F};

//��������ܶ�ѡ��
uchar  code table[]= { 
//0		1	 2     3     4     5     6     7     8
0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,
//9     A     B	   C	 D	   E	 F		H	 L	 
0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x76, 0x38,
//n	   u	  -	  Ϩ��
0x37, 0x3E, 0x40, 0x00 };

/*====================================
����	�� delay(uint z)
����	��z ��ʱ�����趨��ȡֵ��Χ0-65535
����ֵ	����
����	��12T/Fosc11.0592M���뼶��ʱ
====================================*/
void delay(uint z)
{
	uint x,y;
	for(x = z; x > 0; x--)
		for(y = 114; y > 0 ; y--); 		
} 

/*====================================
����	��key_input()
����	����
����ֵ	��KeyValue
����	���������ɨ��
��������KeyValueȫ�ֱ���ֵ������Ӧ�仯
====================================*/
void key_scan(void)
{
	//4*4�������ɨ��
	P1 = 0X0F;//��ɨ��
	if(P1 != 0X0F)//�жϰ����Ƿ񱻰���
	{
		delay(10);//�������10ms
		if(P1 != 0X0F)//�жϰ����Ƿ񱻰���
		{
            P1 = 0X0F;
			switch(P1) //�ж���һ�б�����
			{
                case 0x07:	KeyValue = 0;	break;//��һ�б�����
                case 0x0b:	KeyValue = 1;	break;//�ڶ��б����� 
                case 0x0d:	KeyValue = 2;	break;//�����б�����                
				case 0x0e:	KeyValue = 3;	break;//�����б�����
			}
			P1 = 0XF0;//��ɨ��
			switch(P1) //�ж���һ�б�����
			{
                case 0x70:	KeyValue = KeyValue;    	break;//��һ�б����� 
                case 0xb0:	KeyValue = KeyValue + 4;	break;//�ڶ��б�����
                case 0xd0:	KeyValue = KeyValue + 8;	break;//�����б�����
				case 0xe0:	KeyValue = KeyValue + 12;	break;//�����б�����
			}	
		}
	}
}

void Record_Play_Fun()
{
    //һֱ��ֱ���ٰ���play��
    while(Play == true && Record == false)
    {
        uint n=0;
        uint j=0;
        //�����ֹͣ��һֱ��
        if(melody[index]!=0)
        n = melody[index++]-1;
        if(melody[index]== 0 )
        {
            index=0;
            n = melody[index++]-1;
        }
        TR0 = 1;
        P0 = table[n];          //�����
        P2 = led[n%8];         //��
		FreqTemp = Freqtab[n];//����
        delay(1000);
        TR0 = 0;
    }
    
    if(Play == false && KeyValue <=16)
    {
            FreqTemp = Freqtab[KeyValue];//����
            P1= 0XF0;
            TR0 = 1;
            while(P1 != 0XF0)
            {
                P0 = table[KeyValue];//����ͣ������ܺͷ�����
                P2 = led[KeyValue%8];//��
            }
             if(Record == true)
                 melody[index++] = KeyValue+1; //�������
        TR0 = 0;
    }
    	SPEAK = 1;	//����
        P0 = 0x00;
        P2 = 0xFF;
        P1 = 0X00;
        KeyValue = 20;
}
/*====================================
����	��EX0_INT�жϺ���  �ȶ�ʱ��0���߼�
����	����
����ֵ	����
����	�����������ж�ɨ��
EX0_INT:KEY3 ����   EX1_INT:KEY4  ¼��
====================================*/
void EX0_INT(void) interrupt 0
{
            Play = !Play;
}

 void EX1_INT(void) interrupt 3
 {
     Record = !Record;
     if(Record == true)
     memset(melody, 0, sizeof(melody));                          //��ջ���
     if(Record == false)
     {
     melody[index]=0; 
     index =0;                                                  //��ͷ��ʼ
     }
 }
/*====================================
����	����ʱ��0�жϺ���T0_INT(void) interrupt 1
����	����
����ֵ	��
����	��
��������������
====================================*/
void T0_INT(void) interrupt 1
{
     TL0 = FreqTemp;                //���붨ʱ�����ڵĳ�ʼֵ
     TH0 = FreqTemp >> 8;
     SPEAK = ~SPEAK;                    //����
} 

void main()
{
	TMOD = 0X01;		//��ʱ��T0����Ϊ��ʽ1 
	EA = 1;             //�����ж� 
	ET0 = 1;            //�򿪶�ʱ��T0���ж�����
	TR0 = 0;            //�رն�ʱ��
    IT0=1;              //����EX0_INT�����س�����ʽ��1�½���,0�͵�ƽ��
    IT1=1;                 //����EX1_INT�����س�����ʽ��1�½���,0�͵�ƽ��
    EX1=1;                 //��EX1_INT���ж�����
    EX0=1;              //��EX0_INT���ж�����
    
    LSA=0; //��һ��������ṩλѡ
	LSB=0;
	LSC=0;//�е�С���⣬λѡû�е�λ���ܽŷ���Ҳû���⣬����ֻ������ܵ�С����
    
    SPEAK = 1;	//����
    P0 = 0x00;//û��
    P2 = 0XFF;//�Ʋ���
	while(1)
	{
         delay(3);
		 key_scan();//��ȡ���󰴼�ֵ
        Record_Play_Fun();//��¼\����\����
	}		

}

