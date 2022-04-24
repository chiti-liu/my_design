#include <reg51.h>
#include "string.h"

#define uint unsigned int
#define uchar unsigned char

sbit LSA=P2^2;
sbit LSB=P2^3;
sbit LSC=P2^4;

sbit KEY3 = P3^2;//独立按键K3
sbit KEY4 = P3^3;//独立按键K4

typedef enum {false, true} bool;
bool Record = false;        //录制标志位
bool Play = false;          //播放标志位

uint melody[30]={0};       //曲子   
uint index=0;               //储存的索引
uchar KeyValue = 20;        //按键值显示-

unsigned int code Freqtab[] = {	//定时半周期的初始值
     64021,64103,64260,64400,      //低音3 4 5 6
     64524,64580,64684,64777,      //低音7,中音1 2 3
     64021,64103,64260,64400,      //中音4 5 6 7
     65058,65110,65157,65178};     //高音1 2 3 4

//接受音调
uint FreqTemp;
     
//蜂鸣器
sbit SPEAK = P1^5;
uchar led[] = {0XFE,0XFD,0XFB,0XF7,0XEF,0XDF,0XBF,0X7F};

//共阴数码管段选表
uchar  code table[]= { 
//0		1	 2     3     4     5     6     7     8
0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,
//9     A     B	   C	 D	   E	 F		H	 L	 
0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x76, 0x38,
//n	   u	  -	  熄灭
0x37, 0x3E, 0x40, 0x00 };

/*====================================
函数	： delay(uint z)
参数	：z 延时毫秒设定，取值范围0-65535
返回值	：无
描述	：12T/Fosc11.0592M毫秒级延时
====================================*/
void delay(uint z)
{
	uint x,y;
	for(x = z; x > 0; x--)
		for(y = 114; y > 0 ; y--); 		
} 

/*====================================
函数	：key_input()
参数	：无
返回值	：KeyValue
描述	：矩阵键盘扫描
按键按下KeyValue全局变量值发生相应变化
====================================*/
void key_scan(void)
{
	//4*4矩阵键盘扫描
	P1 = 0X0F;//列扫描
	if(P1 != 0X0F)//判断按键是否被按下
	{
		delay(10);//软件消抖10ms
		if(P1 != 0X0F)//判断按键是否被按下
		{
            P1 = 0X0F;
			switch(P1) //判断那一列被按下
			{
                case 0x07:	KeyValue = 0;	break;//第一列被按下
                case 0x0b:	KeyValue = 1;	break;//第二列被按下 
                case 0x0d:	KeyValue = 2;	break;//第三列被按下                
				case 0x0e:	KeyValue = 3;	break;//第四列被按下
			}
			P1 = 0XF0;//行扫描
			switch(P1) //判断那一行被按下
			{
                case 0x70:	KeyValue = KeyValue;    	break;//第一行被按下 
                case 0xb0:	KeyValue = KeyValue + 4;	break;//第二行被按下
                case 0xd0:	KeyValue = KeyValue + 8;	break;//第三行被按下
				case 0xe0:	KeyValue = KeyValue + 12;	break;//第四行被按下
			}	
		}
	}
}

void Record_Play_Fun()
{
    //一直放直到再按下play键
    while(Play == true && Record == false)
    {
        uint n=0;
        uint j=0;
        //如果不停止就一直放
        if(melody[index]!=0)
        n = melody[index++]-1;
        if(melody[index]== 0 )
        {
            index=0;
            n = melody[index++]-1;
        }
        TR0 = 1;
        P0 = table[n];          //数码管
        P2 = led[n%8];         //灯
		FreqTemp = Freqtab[n];//音调
        delay(1000);
        TR0 = 0;
    }
    
    if(Play == false && KeyValue <=16)
    {
            FreqTemp = Freqtab[KeyValue];//音调
            P1= 0XF0;
            TR0 = 1;
            while(P1 != 0XF0)
            {
                P0 = table[KeyValue];//松手停下数码管和蜂鸣器
                P2 = led[KeyValue%8];//灯
            }
             if(Record == true)
                 melody[index++] = KeyValue+1; //储存歌曲
        TR0 = 0;
    }
    	SPEAK = 1;	//不响
        P0 = 0x00;
        P2 = 0xFF;
        P1 = 0X00;
        KeyValue = 20;
}
/*====================================
函数	：EX0_INT中断函数  比定时器0更高级
参数	：无
返回值	：无
描述	：独立键盘中断扫描
EX0_INT:KEY3 播放   EX1_INT:KEY4  录制
====================================*/
void EX0_INT(void) interrupt 0
{
            Play = !Play;
}

 void EX1_INT(void) interrupt 3
 {
     Record = !Record;
     if(Record == true)
     memset(melody, 0, sizeof(melody));                          //清空缓存
     if(Record == false)
     {
     melody[index]=0; 
     index =0;                                                  //从头开始
     }
 }
/*====================================
函数	：定时器0中断函数T0_INT(void) interrupt 1
参数	：无
返回值	：
描述	：
蜂鸣器播放音乐
====================================*/
void T0_INT(void) interrupt 1
{
     TL0 = FreqTemp;                //载入定时半周期的初始值
     TH0 = FreqTemp >> 8;
     SPEAK = ~SPEAK;                    //发音
} 

void main()
{
	TMOD = 0X01;		//定时器T0设置为方式1 
	EA = 1;             //打开总中断 
	ET0 = 1;            //打开定时器T0的中断允许
	TR0 = 0;            //关闭定时器
    IT0=1;              //设置EX0_INT跳变沿出发方式（1下降沿,0低点平）
    IT1=1;                 //设置EX1_INT跳变沿出发方式（1下降沿,0低点平）
    EX1=1;                 //打开EX1_INT的中断允许。
    EX0=1;              //打开EX0_INT的中断允许。
    
    LSA=0; //给一个数码管提供位选
	LSB=0;
	LSC=0;//有点小问题，位选没有到位，管脚分配也没问题，不过只是数码管的小问题
    
    SPEAK = 1;	//不响
    P0 = 0x00;//没数
    P2 = 0XFF;//灯不亮
	while(1)
	{
         delay(3);
		 key_scan();//获取矩阵按键值
        Record_Play_Fun();//记录\播放\演奏
	}		

}

