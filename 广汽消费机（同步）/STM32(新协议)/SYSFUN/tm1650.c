#include "tm1650.h"
#include "stdbool.h"
#include "head.h"


void dtmelay(unsigned char x)
{
	unsigned char i;
	for(;x>0;x--) for(i=110;i>0;i--);
}
void TM16_SDA_IN(void )		//设置SDA为输入模式
{
	GPIO_InitTypeDef GPIO_INIT;		//设置SDA为输入模式
	GPIO_INIT.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_INIT.GPIO_Pin=TM16_SDA;
	GPIO_INIT.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(TM16_PORT,&GPIO_INIT);
}
void TM16_SDA_OUT(void )			//设置SDA为输出模式
{
	GPIO_InitTypeDef GPIO_INIT;
	GPIO_INIT.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_INIT.GPIO_Pin=TM16_SDA;
	GPIO_INIT.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(TM16_PORT,&GPIO_INIT);
}
void TM16_IIC_Start(void )		//开始信号
{
	TM16_SDA_H;
	TM16_SCL_H;
	TM16_delay(5);
	TM16_SDA_L;
	TM16_delay(5);
	TM16_SCL_L;
}
void TM16_IIC_Stop(void )			//停止信号
{
	TM16_SDA_L;
	TM16_SCL_H;
	TM16_delay(5);
	TM16_SDA_H;
	TM16_delay(5);
	TM16_SCL_L;
}
bool TM16_Read_ACK(void )		//读取应答信号
{
	bool ack;
	TM16_SDA_IN();
	TM16_SCL_H;
	if(TM16_ReadSDA() == SET) ack=true;
	else ack=false;
	TM16_SCL_L;
	TM16_delay(5);
	TM16_SDA_OUT();
	return ack;
}	
void TM16_Send_ACK(bool ack)
{
	TM16_SDA_OUT();
	if(ack == true) TM16_SDA_H;
	else TM16_SDA_L;
	TM16_SCL_H;
	TM16_delay(5);
	TM16_SCL_L;
}

void TM16_Send_Byte(unsigned char byte)			//发送一位数据
{
	unsigned char count;
	TM16_SCL_L;
	for(count=0;count<8;count++)
	{
		if(byte & 0x80) TM16_SDA_H;
		else TM16_SDA_L;
		byte<<=1;
		TM16_delay(2);
		TM16_SCL_H;
		TM16_delay(5);
		TM16_SCL_L;
		TM16_delay(5);
	}
	TM16_Read_ACK();
}
unsigned char TM16_Read_Byte(void )		//读取一位数据
{
	unsigned char byte,count;
	TM16_SDA_IN();
	
	TM16_SDA_H;
	for(count=0;count<8;count ++)
	{
		TM16_SCL_H;
		TM16_delay(5);
		byte<<=1;
		if(TM16_ReadSDA()) 
			byte++;
	//	else byte&=0xfe;
	//	TM16_SDA_L;
		
		TM16_SCL_L;
		TM16_delay(5);
	}
	TM16_SDA_H;
	TM16_delay(5);
	TM16_SCL_H;
	TM16_delay(5);	
	TM16_SCL_L;
	TM16_delay(5);
	
	TM16_SDA_OUT();

	return byte;
}
void TM16_Write_REG(unsigned char reg, unsigned char data)		//写命令，设置
{
	TM16_IIC_Start();
	TM16_Send_Byte(reg);
	TM16_Send_Byte(data);
	TM16_IIC_Stop();
}
void TM16_Set_Brig(unsigned char BRIG)		//设置数码管显示的亮度
{
	unsigned char brig[]={0x11,0x21,0x31,0x41,0x51,0x61,0x71,0x01};
//	if((BRIG>=0) && (BRIG <=7)) 
		
		TM16_Write_REG(0x48,brig[BRIG]);	
//	else 
//		TM16_Write_REG(0x48,brig[0]);
}
void TM16_INIT(unsigned char brig)		//TM165 的初始化  brig为初始化亮度0-7
{
	GPIO_InitTypeDef GPIO_INIT;
	RCC_APB2PeriphClockCmd(TM16_RCC ,ENABLE);
	
	GPIO_INIT.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_INIT.GPIO_Pin=TM16_SDA | TM16_SCL;
	GPIO_INIT.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(TM16_PORT,&GPIO_INIT);
	delay_ms(100);
//	delay_ms(500);
	
	TM16_Set_Brig(brig);
}

//void TM16_Display(unsigned char data,unsigned char di)		//显示函数，data为显示的数据，0-99，di设置是否显示小数点，0为都不显示小数点，1为第一位显示小数点，2为第二位显示小数点，3为第一位和第二位都显示小数点
//{
//	static unsigned char TM16_Display_buf[10]={0x3F,0x6,0x5B,0x4F,0x66,0x6D,0x7D,0x7,0x7F,0x6F};
//	unsigned char ge,shi;
//	ge=(int)data%10;
//	if(!(shi=data/10)) {shi=0;}
//	if(di==1) 
//                 {ge=TM16_Display_buf[ge]+0x80;shi=TM16_Display_buf[shi];}		//第一位显示小数点
//	else if(di==2) {ge=TM16_Display_buf[ge];shi=TM16_Display_buf[shi]+0x80;}	//第二位显示小数点
//	else if(di==3) {ge=TM16_Display_buf[ge]+0x80;shi=TM16_Display_buf[shi]+0x80;}		//两位都显示小数点
//	else {ge=TM16_Display_buf[ge]+0x80;shi=TM16_Display_buf[shi]+0x80;}		//都不显示小数点
//	TM16_Write_REG(0x68,ge);
//	TM16_Write_REG(0x6a,shi);
//}


//***********读按键命令*************************************
unsigned char TM1650_FD650_READ(void)
{
	
unsigned char TM16_key =0;

	TM16_IIC_Start();
	
	TM16_Send_Byte(0x49);
	
	TM16_key=TM16_Read_Byte();
	
	TM16_IIC_Stop();
	
	return TM16_key;
} 



uint8_t  tmKeyscan(void)
{
	uint8_t  key_val = 0;
	
	uint8_t  tmkeyval[2]={0};
	
  static  uint8_t	oldkey[2];
	
	tmkeyval[0] =TM1650_FD650_READ();
	
	/***********************同一张卡只读取一次*********************************/	
	if (MyStrEQ(oldkey, tmkeyval, 1))
		{		
			  return 0;
		}
		
	MyStrCPY(oldkey, tmkeyval, 1);

	 switch(tmkeyval[0])    
		{
			case 0x44: key_val=1;  break;  // 1
			case 0x4C: key_val=2;  break;  // 2
			case 0x54: key_val=3;  break;  // 3
			case 0x5C: key_val=13; break;  // 菜单						
			case 0x64: key_val=17; break;  // 清除	
			
			case 0x45: key_val=4;  break;  // 4
			case 0x4D: key_val=5;  break;  // 5
			case 0x55: key_val=6;  break;  // 6
			case 0x5D: key_val=14; break;  // 上翻
			case 0x65: key_val=18; break;  // 打印
			
			
			case 0x46: key_val=7;  break;  // 7
			case 0x4E: key_val=8;  break;  // 8
			case 0x56: key_val=9;  break;  // 9
			case 0x5E: key_val=15; break;  // 下翻
			case 0x66: key_val=19; break;  // 查询				
			
			case 0x47: key_val=10; break;  // *
			case 0x4F: key_val=11; break;  // 0
			case 0x57: key_val=12; break;  // #
			case 0x5F: key_val=16; break;  // 确认
								
			default:   key_val=0; break;
			
		}		
			
	return key_val;

}


