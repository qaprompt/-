#include "Head.h"
#include <string.h>

/**********************************************************
//变量定义。
**********************************************************/

 uint32_t   comidx;                         //串口接收数据个数 
 
 uint8_t    endcount;                       //结束标志  ~  计数
 
 uint8_t    endcount_4g;                    //结束标志  ~  计数
 
 uint8_t    combuff[COM_SIZE];              //串口接收缓冲区 
 
 uint8_t    communication_data[COM_SIZE];   //通讯缓冲区 
 
 uint32_t   Data_length;                    //数据长度

//static  u8   order_value[]={0xAD,0xF8,0x54,0x59,0x10};							// 返回值命令头

/**********************************************************
//固件版本！
**********************************************************/

uint8_t  VERSION_SIZE = 10;
uint8_t  SOFT_VERSION[] = "2019-04-24";

//机器型号
uint8_t  TYPE_SIZE = 11;
uint8_t  MACH_TYPE[] = "KT2169-YB-T";

/**********************************************************
//常量定义
**********************************************************/

uint8_t  COM_ERROR[] = {0x2A,0xAA,0xAA,0xAA,0xAA,0xBB,0xBB,0x00,0x00,0x00,0x7E,0x7E,0x7E};   //串口返回错误提示

uint8_t  Return_Codebuf[10]={0x4F,0x4B,0X45,0X52,0X52,0X31};    //回写返回代码

uint8_t  Return_Endvalue[3]={0x7E,0x7E,0x7E};							// 返回尾

uint8_t  Retainbitbuf[4] ={0xFF,0XFF,0XFF,0XFF};        //命令保留字节

/**********************************************************

**********************************************************/

/**
返回值发送
**/
void ReyurnCommand_Send(uint8_t *data ,uint16_t len)
{

	if (combuff[10]==0XA1)	
		 { RS485_Send_Data(data, len);}
	 
  else	if (RxDataBuff[10]==0XA4)	
		 { EC20_SendString(data, len);}	 
		 
  else	if (TcpServer_databuff[10]==0XA2)	
		 { Write_SOCK_Data_Buffer(1, data, len );	 }	 						     	         /*向Cilent发送数据*/
						 
  else	if (TcpClient_databuff[10]==0XA3)	
		 { Write_SOCK_Data_Buffer(0, data, len );	 }							     	         /*向Server发送数据*/

	LED1_fun(1, SHORT_BEEP);
 //  UARTSUCCESS();                  //蜂鸣器发声
}


	
	
	/**
发送获取命令的返回值 
	
注意：	最多不超过255字节长度
	
	@valuelen   数据长度 
	@data       数据内容
		
**/
void Send_ReyurnGETCommand_Value( uint8_t *data ,uint16_t len )
{
	
	
	communication_data[AGR_LEN_POS]   = (len>>8)&0XFF;         //数据长度		
	communication_data[AGR_LEN_POS+1] = len&0XFF;       //数据长度;       //数据长度
	
	MyStrCPY(communication_data + AGR_DATA_POS, data, len);//数据
		  
	communication_data[AGR_ROX_POS + len -1] = GetXOR(communication_data + AGR_DATA_POS, len);//xor检验位
				
  MyStrCPY(communication_data + AGR_RETAIN_POS +len -1, Retainbitbuf, 4);//保留字节
	
	MyStrCPY(communication_data +AGR_END_POS + len, Return_Endvalue, AGR_END_SIZE);//码尾
	
  ReyurnCommand_Send(communication_data , AGR_MAXEND_POS + len);	
	
	UARTSUCCESS();                  //蜂鸣器发声
}
	
/**********************************************************
//读取机器型号  OK
读取:  2A 18 03 24 00 01 51 05 07 01 A1 01 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 01 00 0B 4B 54 32 31 36 39 2D 57 49 46 49 2F FF FF FF FF 7E 7E 7E

**********************************************************/
void GetMachType(void)
{

Send_ReyurnGETCommand_Value( MACH_TYPE ,TYPE_SIZE);
	
} 

/**********************************************************
//读取版本信息 OK
读取:  2A 18 03 24 00 01 51 05 07 01 A1 02 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 02 51 05 07 01 A1 02 00 0A 32 30 31 37 2D 30 35 2D 30 34 05 FF FF FF FF 7E 7E 7E

**********************************************************/
void GetVersion(void)
{	  
  Send_ReyurnGETCommand_Value( SOFT_VERSION ,VERSION_SIZE);	
} 


/**********************************************************
//设置机号。 OK
发送：2A 18 03 24 00 01 51 05 07 01 A1 03 00 04 01 02 03 04 04 FF FF FF FF 7E 7E 7E
应答：2A 18 03 24 00 01 51 05 07 01 A1 03 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

**********************************************************/
void SetMachno(void)
{
	if (communication_data[AGR_LEN_POS2] != 0x04) //检验数据长度 
	  	{				
				Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写						
				return;
			}	
	SaveMachno(communication_data +AGR_DATA_POS);    //写机号	 
			
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写	
		
}
/**********************************************************
//读取机号。 0K
发送：2A 18 03 24 00 01 51 05 07 01 A1 04 00 00 00 FF FF FF FF 7E 7E 7E
应答：2A 18 03 24 00 01 51 05 07 01 A1 04 00 04 01 02 03 04 04 FF FF FF FF 7E 7E 7E

**********************************************************/
void GetMachno(void)
{
			
	Send_ReyurnGETCommand_Value( machno ,MACH_SIZE);	

}

/*******************************************************************************

设置时间  ok

发送：   2A 18 03 26 00 01 51 05 07 01 A1 05 00 06 18 03 26 09 36 30 32 FF FF FF FF 7E 7E 7E
应答：   2A 18 03 26 00 01 51 05 07 01 A1 05 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/
void SetTime(void)
{
	if (communication_data[AGR_LEN_POS2] != 0x06) //检验数据长度 
	  	{
        Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写		
				
				Send_ReyurnGETCommand_Value( machno ,MACH_SIZE);	
				
			 return;
			}
	
	SetNowTime(communication_data + AGR_FIXED_SIZE);     //设置时间
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写	
			
}



/*******************************************************************************
读取时间   ok
发送:  2A 18 03 24 00 01 51 05 07 01 A1 06 00 00 00 FF FF FF FF 7E 7E 7E  
应答： 2A 18 03 24 00 01 51 05 07 01 A1 06 00 07 18 03 26 09 38 08 01 05 FF FF FF FF 7E 7E 7E
*******************************************************************************/
void GetTime(void)
{
	uint8_t systime[8];
	uint8_t time_len = 7; 
		
  DS1302_Get_sysTime(systime) ;
	
  Send_ReyurnGETCommand_Value(systime , time_len);    //回写		
	
}  

/**********************************************************
//设置读写扇区与密码  OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 07 00 07 01 FF FF FF FF FF FF 01 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 07 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

**********************************************************/
void SetSectionAndPassword(void)
{
	if (communication_data[AGR_LEN_POS2] != (PASSWORD_SIZE + 1)) 
	{
			 Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写	
		
			return;
	}

	SetPassword(communication_data + AGR_FIXED_SIZE , 0);
	
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}  


/**********************************************************
//读取读写扇区与密码   OK
发送:   2A 18 03 24 00 01 51 05 07 01 A1 08 00 00 00 FF FF FF FF 7E 7E 7E
应答：  2A 18 03 24 00 01 51 05 07 01 A1 08 00 07 01 FF FF FF FF FF FF 01 FF FF FF FF 7E 7E 7E

**********************************************************/
void GetSectionAndPassword(void)
{
  u8 isec[7] = {0X01};
		
	AT24CXX_Read(SECTION_POS, isec , 7);	
	
  Send_ReyurnGETCommand_Value(isec , 7);    //回写	
} 


/*******************************************************************************
设置打卡延时  OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 09 00 01 01 01 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 09 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_Clock_delay(void)
{

	if (communication_data[AGR_LEN_POS2] != 0X01) 
	{
		Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写	
		
		return;
	}
	
  AT24CXX_Write(CLOCK_DELAY, communication_data + AGR_FIXED_SIZE , 1);
	
	AT24CXX_Read(CLOCK_DELAY, repea_time , 1);   //刷卡延时
		
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}

/*******************************************************************************
设置继电器延时   OK

发送:  2A 18 03 24 00 01 51 05 07 01 A1 0A 00 01 03 03 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 0A 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_Output_delay(void)
{

	if (communication_data[AGR_LEN_POS2] != 0X01) 
	{
		Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写			
		return;
	}
	
  AT24CXX_Write(REL_OUTTIME_IDX, communication_data + AGR_FIXED_SIZE, 1);
	
	AT24CXX_Read(REL_OUTTIME_IDX,  output_time , 1);   //继电器输出延时
	
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}


/*******************************************************************************
设置是否开启白名单   OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 0B 00 01 BB BB FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 0B 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_list_enabled(void)
{

	if (communication_data[AGR_LEN_POS2] != 0X01) 
	{
    Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写	
		
		return;
	}
	
  AT24CXX_Write(WHITEYELL,communication_data + AGR_FIXED_SIZE, 1);
	
	AT24CXX_Read(WHITEYELL,  whiteallow , 1);   //是否白名单检测
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}


/*******************************************************************************
读取机器基本参数   OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 0C 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 0C 00 0B 03 FF 01 FF BB FF FF FF FF FF FF B9 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Get_sysvar(void)
{
  u8 sysvarbuff[SYSVAR_SIZE];
	
	AT24CXX_Read(SYSVAR_ST, sysvarbuff , SYSVAR_SIZE );
		  
	Send_ReyurnGETCommand_Value(sysvarbuff , SYSVAR_SIZE);      //返回值发送		

}

/*******************************************************************************
设置机器恢复出厂设置   OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 0D 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 0D 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_Restore_factory(void)
{

	LoadDefaultValue();  //恢复出厂设置
	
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
	
	delay_ms(200);
	
	reboot();       

}


/*******************************************************************************
设置机器网络恢复出厂设置  OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 0E 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 0E 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_ResNottore_factory(void)
{
	if (communication_data[AGR_CMD_POS]==0xA3)
		{
		 LoadnNetDefaultValue();   //恢复网络出厂设置  TCP
		}
	
	else if (communication_data[AGR_CMD_POS]==0xA4)
		{
		 LoadnEC20DefaultValue();   //恢复网络出厂设置  4G
		}
		
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
	
	delay_ms(200);
		
	
	if (communication_data[AGR_CMD_POS]==0xA3)
	{
		reboot();                         //STM32系统软复位函数(重启系统)	
	}

	else if (communication_data[AGR_CMD_POS]==0xA4)
		{
			QuectelEC20_Rest();		            //复位模块	
		}	
		
		
	
}


/*******************************************************************************
读取CPU信息    返回值待定
发送:  2A 18 03 24 00 01 51 05 07 01 A1 0F 00 00 00 FF FF FF FF 7E 7E 7E
应答： 43 04 55 19 32 37 50 33 D3 68 FF 37 2D 2D 01 00 2D 2D 4D 61 72 20 32 34 20 32 30 31 38 2D 31 37 3A 31 36 3A 33 39
*******************************************************************************/

void Get_ChipInfo_Com(void)
{

	Get_ChipInfo();
	
//	EchoCommand();      //回写
	UARTSUCCESS();      //蜂鸣器发声
}


/*******************************************************************************
设置tcp模式    //此功能暂时未使用    返回值待定
发送:  2A 18 03 24 00 01 51 05 07 01 A1 21 00 06 53 74 61 74 69 63 38 FF FF FF FF 7E 7E 7E    //静态IP
应答： 
*******************************************************************************/

void Set_TCP_Mode(void)
{
	u8 DHCP_modebuff[6]={0x44, 0x48, 0x43, 0x50,  0x00,  0x00};
	
	if (communication_data[AGR_LEN_POS2] != 0x06) //检验数据长度 
	  	{
         Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
				return;
			}
    			
	AT24CXX_Write(TCP_STATIC_DHCP_POS,communication_data +AGR_FIXED_SIZE,6);	   //24C512写模式参数
			
  if(MyStrEQ(DHCP_modebuff,communication_data +AGR_FIXED_SIZE,6))
	{	
   Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
			
   delay_ms(200);		  //延时200毫秒
		
	 reboot();           //STM32系统软复位函数(重启系统)
	}
		
	else
		{
			Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
		}	
}


/*******************************************************************************
设置本机Mac地址   OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 22 00 06 00 08 DC 11 11 11 C5 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 22 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_This_MAC(void)
{

	if (communication_data[AGR_LEN_POS2] != 0x06) //检验数据长度 
	  	{
				Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
				return;
			}
    			
	AT24CXX_Write(TCP_THIS_MAC_POS,communication_data+AGR_FIXED_SIZE,6);	   //24C512写Mac
			
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
	
	delay_ms(200);		  //延时200毫秒
			
	reboot();           //STM32系统软复位函数(重启系统)
			
}

/*******************************************************************************
读取本机Mac地址   OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 23 00 00 00 FF FF FF FF 7E 7E 7E             
应答： 2A 18 03 24 00 01 51 05 07 01 A1 23 00 06 00 08 DC 11 11 11 C5 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Get_This_MAC(void)
{
  u8 Macbuff[6]={0X00};
	
	AT24CXX_Read(TCP_THIS_MAC_POS,Macbuff,6);	   //24C512读取Mac地址
		
  Send_ReyurnGETCommand_Value(Macbuff ,6);    //回写		

}

/*******************************************************************************
设置tcp本地IP     OK

发送: 2A 18 03 24 00 01 51 05 07 01 A1 24 00 0C C0 A8 00 7B FF FF FF 00 C0 A8 00 01 85 FF FF FF FF 7E 7E 7E    192.168.0.123 255 255 255 0 192 168 0 1
      2A 18 03 24 00 01 51 05 07 01 A1 24 00 0C C0 A8 00 7D FF FF FF 00 C0 A8 00 01 83 FF FF FF FF 7E 7E 7E    192.168.0.125 255 255 255 0 192 168 0 1

应答： 2A 18 03 24 00 01 51 05 07 01 A1 24 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E 
*******************************************************************************/

void Set_TCP_Local_IP(void)
{

	if (communication_data[AGR_LEN_POS2] != 0X0C) //检验数据长度 
	  	{
				Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
				return;
			}
    			
	AT24CXX_Write(TCP_LOCAL_IP_POS,communication_data +AGR_FIXED_SIZE,12);	  //24C512写IP
	
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
			
  delay_ms(500);		                                                  //延时1秒
			
	reboot();                                                           //STM32系统软复位函数(重启系统)
	
}


///*******************************************************************************
//读取tcp本地IP  OK
//发送:  2A 18 03 24 00 01 51 05 07 01 A1 25 00 00 00 FF FF FF FF 7E 7E 7E
//应答： 2A 18 03 24 00 01 51 05 07 01 A1 25 00 12 00 08 DC 11 11 11 C0 A8 00 7B FF FF FF 00 C0 A8 00 01 40 FF FF FF FF 7E 7E 7E
//*******************************************************************************/

void Get_TCP_Local_IP(void)
{
  uint8_t  TCP_Localbuf[18]={0};
	
	MyStrCPY(TCP_Localbuf,     Phy_Addr,   6);                         //拷贝数据  拷贝Mac地址  
	MyStrCPY(TCP_Localbuf +6,  IP_Addr,    4);                         //拷贝数据  拷贝本地IP 
	MyStrCPY(TCP_Localbuf +10, Sub_Mask,   4);                         //拷贝数据  拷贝子网掩码 
	MyStrCPY(TCP_Localbuf +14, Gateway_IP, 4);                         //拷贝数据  拷贝网关
	
	Send_ReyurnGETCommand_Value(TCP_Localbuf , 18);    //回写
}


/*******************************************************************************
设置tcp远端IP和端口号    OK
发送:  2A 18 03 24 00 01 51 05 07 01 A3 26 00 06 C0 A8 00 65 30 3a 07 FF FF FF FF 7E 7E 7E            192.168.0.101  6000
       2A 18 03 24 00 01 51 05 07 01 A3 26 00 06 C0 A8 00 6D 00 35 30 FF FF FF FF 7E 7E 7E            192.168.0.109  53
       2A 18 03 24 00 01 51 05 07 01 A3 26 00 06 C0 A8 00 67 30 3A 05 FF FF FF FF 7E 7E 7E            192.168.0.103  12346
       2A 18 03 24 00 01 51 05 07 01 A3 26 00 06 C0 A8 00 93 30 3A f1 FF FF FF FF 7E 7E 7E            192.168.0.147  12346
       2A 18 03 24 00 01 51 05 07 01 A3 26 00 06 C0 A8 00 65 30 3a 07 FF FF FF FF 7E 7E 7E            192.168.0.101  12346
       2A 18 03 24 00 01 51 05 07 01 A3 26 00 06 C0 A8 00 6A 30 3a 08 FF FF FF FF 7E 7E 7E            192.168.0.106  12346
       2A 18 03 24 00 01 51 05 07 01 A3 26 00 06 C0 A8 00 64 30 3a 06 FF FF FF FF 7E 7E 7E            192.168.0.100  12346

应答： 2A 18 03 24 00 01 51 05 07 01 A1 26 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_TCP_Remote_IP(void)
{

	if (communication_data[AGR_LEN_POS2] != 0x06) //检验数据长度 
		{
			Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
							
			return;
		}
				
	AT24CXX_Write(TCP_REMOTE_IPandPort_POS,communication_data +AGR_FIXED_SIZE,6);	              //24C512写IP
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
			
  delay_ms(200);		                                                  //延时2000毫秒
			
	reboot();                                                           //STM32系统软复位函数(重启系统)
	
}


///*******************************************************************************
//读取tcp远端IP和端口号   OK
//发送:  2A 18 03 24 00 01 51 05 07 01 A1 27 00 00 00 7E 7E 7E
//应答： 2A 18 03 24 00 01 51 05 07 01 A1 27 00 06 C0 A8 00 65 30 3A 07 FF FF FF FF 7E 7E 7E
//*******************************************************************************/

void Get_TCP_Remote_IP(void)
{

	uint8_t  TCP_Remotebuf[6]={0};
	
	MyStrCPY(TCP_Remotebuf,    S0_DIP, 4);                         //拷贝数据  拷贝远端IP
	MyStrCPY(TCP_Remotebuf+4, S0_DPort, 2);                         //拷贝数据  拷贝端口号
		
  Send_ReyurnGETCommand_Value(TCP_Remotebuf , 6);    //回写
}

///*******************************************************************************
//设置4g远端IP和端口号   
//发送:  2A 18 03 24 00 01 51 05 07 01 A1 28 00 00 00 7E 7E 7E
//应答： 2A 18 03 24 00 01 51 05 07 01 A1 28 00 06 C0 A8 00 65 30 3A 07 FF FF FF FF 7E 7E 7E
//*******************************************************************************/

void Set_TCP4G_Remote_IP(void)
{
	if (communication_data[AGR_LEN_POS2] != 0x11) //检验数据长度 
	{
		Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
						
		return;
	}
				
	AT24CXX_Write(TCP4G_REMOTE_IPandPort_POS,communication_data +AGR_FIXED_SIZE,17);	              //24C512写IP
	
	Init_EC20_Remote_IP();                                                      //初始化IP
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
			
  delay_ms(200);		                                                  //延时2000毫秒
			
	QuectelEC20_Rest();		            //复位模块	
	
	//reboot();                                                           //STM32系统软复位函数(重启系统)

}

///*******************************************************************************
//读取4g远端IP和端口号   
//发送:  2A 18 03 24 00 01 51 05 07 01 A1 29 00 00 00 7E 7E 7E
//应答： 2A 18 03 24 00 01 51 05 07 01 A1 29 00 06 C0 A8 00 65 30 3A 07 FF FF FF FF 7E 7E 7E
//*******************************************************************************/

void Get_TCP4G_Remote_IP(void)
{
	uint8_t  TCP_Remotebuf[18]={0};
	
	MyStrCPY(TCP_Remotebuf,ec20_remoteipbuf, 17);                         //拷贝数据  拷贝远端IP端口号
		
  Send_ReyurnGETCommand_Value(TCP_Remotebuf , 17);    //回写
	
}


/*******************************************************************************
设置开门   OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 2E 00 01 01 01 FF FF FF FF 7E 7E 7E 
应答： 2A 18 03 24 00 01 51 05 07 01 A1 2E 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_Open_Door(void)
		{
			
		if (communication_data[AGR_LEN_POS2] != 0x01) //检验数据长度 
	  	{
			  Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
				return;
			}
			
//		OLEDPasswordZQ();			 	 //OLED显示扇区密码正确		
			
		if(communication_data[AGR_FIXED_SIZE] == 0x01)				
		{
		  BeginOutput1();           //打开继电器1
			
			
					SYN6288_Send(SYN6288CMD_VOICE_PLAY ,SYN6288CMDPARAM_PLAY_MUSIC0,"请进" ,4);
			
			 showStandby=0;
			
				ClrScreen();  //清屏	
     	delay_ms(5);
			
						SetBG_Color(LCD_DARK);
				
				SetDisplayInvert(0);
				
				SetFontType((LCD_ASC16_FONT<<4)|LCD_HZK32_FONT);
				
				SetFG_Color(LCD_WHITE);
			
			  PutString (100  ,25,"请进1");
			
		} 	
		if(communication_data[AGR_FIXED_SIZE] == 0x02)				
		{
		  BeginOutput2();           //打开继电器2
			
       SYN6288_Send(SYN6288CMD_VOICE_PLAY ,SYN6288CMDPARAM_PLAY_MUSIC0,"请进" ,4);
			
			 showStandby=0;
			
				ClrScreen();  //清屏	
	     delay_ms(5);
			SetBG_Color(LCD_DARK);
				
				SetDisplayInvert(0);
				
				SetFontType((LCD_ASC16_FONT<<4)|LCD_HZK32_FONT);
				
				SetFG_Color(LCD_WHITE);
			
			  PutString (100  ,25,"请进2");
			
			
		} 	

		if(communication_data[AGR_FIXED_SIZE] == 0x03)				
		{
			BeginOutput1();           //打开继电器1
			
		  BeginOutput2();           //打开继电器2
			
			SYN6288_Send(SYN6288CMD_VOICE_PLAY ,SYN6288CMDPARAM_PLAY_MUSIC0,"请进" ,4);
			
			 showStandby=0;
			
				ClrScreen();  //清屏	
	     delay_ms(5);
			
				SetBG_Color(LCD_DARK);
				
				SetDisplayInvert(0);
				
				SetFontType((LCD_ASC16_FONT<<4)|LCD_HZK32_FONT);
				
				SetFG_Color(LCD_WHITE);
			
			  PutString (100  ,25,"请进1、2");
			
			
		} 		
		
		Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
		}



/*******************************************************************************
设置不开门   OK
发送:  2A 18 03 24 00 01 51 05 07 01 A1 2F 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 2F 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_NO_Open_Door(void)
		{

	//	OLEDPasswordREE();       //OLED显示扇区密码错误
	//	ERR_BEEP()	;	
			
		Beep(5, SHORT_BEEP);	
			
		Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
			
		}


/*******************************************************************************
实时上传刷卡记录   
发送:  2A 18 03 24 00 01 51 05 07 01 A1 30 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 30 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Realtime_Upload_Record(void)
		{
			
			
			
			
			
		}

/*******************************************************************************
实时上传刷卡返回   
发送:  2A 18 03 24 00 01 51 05 07 01 A1 31 00 04 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 31 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Realtime_Upload_Return(void)
		{
			
			if (communication_data[AGR_LEN_POS2] != 0X1E) 
			{
				Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写	
				
				return;
			}
			
			
//#ifdef   WISDOM_PLATE    //是智盘
//			WP_updatareturnfalg =1;             //实时上传返回标志
      updatareturnfalg =1;                //实时上传返回标志						
//#else					
//			updatareturnfalg =1;                //实时上传返回标志			
//#endif				
			
			
//			DelUPloadRecord(communication_data+AGR_FIXED_SIZE);
				
			MyStrCPY(Server_Returndata, communication_data+AGR_FIXED_SIZE, 30);      //拷贝服务器返回的数据	
			
//         Write_SOCK_Data_Buffer(0, Server_Returndata, 30 );               // 测试用		
//         Write_SOCK_Data_Buffer(1, Server_Returndata, 30 );               // 测试用		
			
	 //   RS485_Send_Data(Server_Returndata , 30);                //
			
		}		
		

/*******************************************************************************
获取实时上传的刷卡数量   
发送:  2A 18 03 24 00 01 51 05 07 01 A1 32 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 32 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void GetUpdataRecordCount(void)
		{
			
			union FourByte maxrecordnum;

			union FourByte allnum;

			u8 RecordNUMbuf[8]={0,0,0,0};        //名单计数数量 前4字节可存储最大数量  后4字节已经存储名单数量
			
			maxrecordnum.i= UPDATA_MAX_RECORD;      //存储最大记录数量

			allnum.i      = rdrealtime_up_count;     //当前存储的记录数
			
			turn_4_data(RecordNUMbuf ,maxrecordnum.c);
			
			turn_4_data(RecordNUMbuf+4 ,allnum.c);
			
			Send_ReyurnGETCommand_Value(RecordNUMbuf , 8);    //回写
			
			
		}		
				
/*******************************************************************************
读取全部实时上传全部刷卡记录   
发送:  2A 18 03 24 00 01 51 05 07 01 A1 33 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 33 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void GetUpdata_MaxRecord(void)
		{
			
	  uint16_t i;
		uint16_t j=1;
		uint16_t namenum=0;	

	  union  TwoByte   package_cycle={0};         //包编号
	
		if(rdrealtime_up_count==0)
		{
		
		  Send_ReyurnGETCommand_Value(Return_Codebuf , 0);    //回写
		
		  return;
		}
		
		namenum =rdrealtime_up_count /64 +1;
		
		for(i=W25X_PAGE_NUM;i<(namenum+W25X_PAGE_NUM);i++)
			{
			IWDG_Feed();//喂狗
				
			package_cycle.i = j++;
			
			communication_data[AGR_LEN_POS]   = 0X07;                  //数据长度		
			communication_data[AGR_LEN_POS2]  = 0X82 ;                 //数据长度
																
			communication_data[AGR_LEN_POS2+1]   = package_cycle.c[1];                  //拷贝包编号	
			communication_data[AGR_LEN_POS2+2]   = package_cycle.c[0] ;                 //拷贝包编号
			
			W25X_Read_Page(communication_data + AGR_FIXED_SIZE +2 ,i);         			//拷贝数据  从Flash中读取	
										
			communication_data[AGR_ROX_POS + W25X_PAGE_SIZE +1] = GetXOR(communication_data + AGR_DATA_POS, W25X_PAGE_SIZE +2);//xor检验位
						
			MyStrCPY(communication_data + AGR_RETAIN_POS +W25X_PAGE_SIZE +1, Retainbitbuf, 4);//保留字节
			
			MyStrCPY(communication_data +AGR_END_POS + W25X_PAGE_SIZE +2, Return_Endvalue, AGR_END_SIZE);//码尾
			
			ReyurnCommand_Send(communication_data ,AGR_MAXEND_POS +W25X_PAGE_SIZE +2 ) ;   //返回值发送		
					
			}

			UARTSUCCESS();
						
		}		
		
	/*******************************************************************************
清除实时上传数据  
发送:  2A 18 03 24 00 01 51 05 07 01 A1 34 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 34 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void ClearUpdataRecord(void)
		{
			
		Del_UpdataRecord();
			
		Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写	
			
		}	
		
	

	/*******************************************************************************
清除实时上传数据测试
发送:  2A 18 03 24 00 01 51 05 07 01 A1 35 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 35 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void UpdataRecordTEST(void)
		{
			
			
		AT24CXX_Write(VOLUME_IDX, communication_data + AGR_FIXED_SIZE , 1);
			
	//	AT24CXX_Write1( VOLUME_IDX  ,keybuff , 1);
			
		InitVolume();                   //初始化音量大小	
			
		Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写		
  //    ReyurnCommand_Send(temporarydata ,30 *rdtemporary_count ) ;   //返回值发送		
			
		}		

	/*******************************************************************************
发送心跳包
发送:  
应答： 
*******************************************************************************/
void Get_Heartbeat_Send(void)
		{
			
			Heartbeat_Send(4);
			
		}
		
	/*******************************************************************************
发送心后返回
发送:  
应答： 
*******************************************************************************/
void Heartbeat_Return(void)
		{
			
			Init_Heartbeat();                //发送心跳变量初始化
			
			ec20_device_info.netWork =1;
		
		}	

	/*******************************************************************************
网络同步时间
发送:  
应答： 
*******************************************************************************/
void Network_Synchronization_Time(void)
		{
	u8 New_timebuf[22]={  0x2A,0x18,0x04,0x18,0x00,0x01,0x51,0x05,0x07,0x01,
		                      0xA4,0x39,0x00,0x00,0X00, 
	                        0xFF,0xFF,0xFF,0xFF,0x7E,0x7E,0x7E	};			

		MyStrCPY(New_timebuf + AGR_MACH_POS, machno, MACH_SIZE);//机号		
													  
//	  Power_Signalbuf[AGR_ROX_POS + 10 -1] = GetXOR(Power_Signalbuf + AGR_DATA_POS, 10);//xor检验位
			
													  
//	  New_timebuf[AGR_ROX_POS + 4 -1] = GetXOR(New_timebuf + AGR_DATA_POS, 4);//xor检验位
		
										
		if   ((COMM_MODE>>0)&0x01)			
		{
			New_timebuf[10] =0xA1;
			RS485_Send_Data(New_timebuf, AGR_MAXEND_POS );		
		}
		 if  ((COMM_MODE>>1)&0x01)
		{
			New_timebuf[10] =0xA2;
			Write_SOCK_Data_Buffer(1,New_timebuf, AGR_MAXEND_POS );
		}
		 if  ((COMM_MODE>>2)&0x01)
		{
			New_timebuf[10] =0xA3;
			Write_SOCK_Data_Buffer(0,New_timebuf, AGR_MAXEND_POS );
		}
		 if  ((COMM_MODE>>3)&0x01)
		{
			New_timebuf[10] =0xA4;

			EC20_SendString(New_timebuf, AGR_MAXEND_POS );
		}

													
//			ReyurnCommand_Send(New_timebuf ,22);										
													
													
		}			
		
		
	/*******************************************************************************
发送开机信号
发送:  
应答： 
*******************************************************************************/
void Send_Power_Signal(void)
		{
			
	u8 Power_Signalbuf[]={  0x2A,0x18,0x04,0x18,0x00,0x01,0x51,0x05,0x07,0x01,
		                      0xA4,0x38,0x00,0x0A,0X4F,0X50,0X45,0X4E,0X53,0X54,
		                      0X53 ,0X54 ,0X45 ,0X4D,0XEE, 
	                        0xFF,0xFF,0xFF,0xFF,0x7E,0x7E,0x7E	};
	
		MyStrCPY(Power_Signalbuf + AGR_MACH_POS, machno, MACH_SIZE);//机号		
													  
	  Power_Signalbuf[AGR_ROX_POS + 10 -1] = GetXOR(Power_Signalbuf + AGR_DATA_POS, 10);//xor检验位
		
		EC20_SendString(Power_Signalbuf, AGR_MAXEND_POS + 10);
												
//		if   ((COMM_MODE>>0)&0x01)			
//		{
//			Power_Signalbuf[10] =0xA1;
//			RS485_Send_Data(Power_Signalbuf, AGR_MAXEND_POS + 10);		
//		}
//		 if  ((COMM_MODE>>1)&0x01)
//		{
//			Power_Signalbuf[10] =0xA2;
//			Write_SOCK_Data_Buffer(1,Power_Signalbuf, AGR_MAXEND_POS + 10);
//		}
//		 if  ((COMM_MODE>>2)&0x01)
//		{
//			Power_Signalbuf[10] =0xA3;
//			Write_SOCK_Data_Buffer(0,Power_Signalbuf, AGR_MAXEND_POS + 10);
//		}
//		 if  ((COMM_MODE>>3)&0x01)
//		{
//			Power_Signalbuf[10] =0xA4;

//			EC20_SendString(Power_Signalbuf, AGR_MAXEND_POS + 10);
//		}
		
		}			
		
		
/************************************白名单*******************************************/
/*******************************************************************************
读取黑白名单数量     OK
		
发送:  2A 18 03 24 00 01 51 05 07 01 A1 10 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 10 00 04 00 00 00 00 00 FF FF FF FF 7E 7E 7E		
*******************************************************************************/
void GetBackWhiteNamesize(void)
{	
	uint8_t  BackWhiteNamebuf[4]={0x00};
	
	u8 i;
	
  u8  Whitenamebuff[72]={0X00};
	
	u8  Backnamebuff[64]={0X00};

	union TwoByte Whitenamecount={0};
	
	union TwoByte Backnamecount={0};
	 
	AT24CXX_2_Read(NUM_START, Whitenamebuff,72);
	
	AT24CXX_Read(BACK_NUM_START, Backnamebuff,64);	
	
	for(i=0;i<72;i=i+2)
	{
		Whitenamecount.i = (Whitenamebuff[i] * 256 + Whitenamebuff[i+1] + Whitenamecount.i);
				
	}	
	for(i=0;i<64;i=i+2)
	{
		Backnamecount.i = (Backnamebuff[i] * 256 + Backnamebuff[i+1] + Backnamecount.i);
	}
	
  BackWhiteNamebuf[0] = Whitenamecount.c[1];     //名单字节数	
  BackWhiteNamebuf[1] = Whitenamecount.c[0];
  BackWhiteNamebuf[2] = Backnamecount.c[1];     //名单字节数	
  BackWhiteNamebuf[3] = Backnamecount.c[0];
		
	Send_ReyurnGETCommand_Value(BackWhiteNamebuf ,4) ;   //返回值发送	

}


/*******************************************************************************
下发全部白名单
发送 ： 2A 18 03 24 00 01 51 05 07 01 A1 11 00 06 12 34 56 78 FF FF 08 FF FF FF FF 7E 7E 7E
应答 ： 2A 18 03 24 00 01 51 05 07 01 A1 11 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/
void SetWhiteGroupName(void)
{	
	uint32_t i=0;
	
	uint32_t NAME_count=0;
	
	union TwoByte Group_addr={0};
	
	NAME_count = Data_length / ( NAME_SIZE+2 );										//算出下发名单数量									
	
	for(i=0;i<NAME_count;i++)
	{		 	
		curgroup = GetSum(communication_data+AGR_FIXED_SIZE+i*(NAME_SIZE+2), NAME_SIZE) % GROUP_NUM;		//名单归属组号（散列因子）
		
		groupcount = GetGroupCount(); 							//获取组名单数量
		/*--------------------------------------------------------------------*/
		if (groupcount < GROUP_RECORD) 							//分组未满存储
		{		
			Group_addr.i = GetGroupAddress();									//组存储起始地址
		
			Group_addr.i =Group_addr.i+ (groupcount*(NAME_SIZE+2));					//组存储地址接尾
			
			if(((128-(Group_addr.i %128))/6)<=0)
			{AT24CXX_2_Write1(Group_addr.i,communication_data +AGR_FIXED_SIZE +i*(NAME_SIZE+2),(NAME_SIZE+2) );	}		//写入名单
			
			else
			{
			 AT24CXX_2_Write(Group_addr.i,communication_data +AGR_FIXED_SIZE +i*(NAME_SIZE+2),(NAME_SIZE+2) );			//写入名单
			}
						
			groupcount+=1;														//名单+1
			
			SaveGroupCount();													//保存组名单数量
				
		}
		/*--------------------------------------------------------------------*/
		else																 	//分组已满存储
		{ 
			commcount = GetCommCount();											//读取公共组名单数量
			
			if(commcount < COMM_SIZE)											//公共区存储未满
			{
				Group_addr.i = COMM_ST;											//公共组存储起始地址
					
				Group_addr.i = Group_addr.i + (commcount* (NAME_SIZE+2));			//公共组存储地址接尾
				
			  if(((128-(Group_addr.i %128))/6)<=0)
			   {AT24CXX_2_Write1(Group_addr.i,communication_data + AGR_FIXED_SIZE + i*(NAME_SIZE+2),(NAME_SIZE+2) );	}		//写入名单
			
				else					
				 {AT24CXX_2_Write(Group_addr.i,communication_data + AGR_FIXED_SIZE + i*(NAME_SIZE+2),(NAME_SIZE+2) );}		//写入名单
				
				commcount+=1;													//名单+1
				
				SaveCommCount();												//保存公共组组名单数量					
			}
			else																//公共区存储已满
		    {			  			
					
				 return;
			}
		}
		/*--------------------------------------------------------------------*/
	}
//	SaveCommCount();												//保存公共组组名单数量		
	
//	SaveGroupCount();													//保存组名单数量
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}


/*******************************************************************************
白名单读取：  OK
发送：  2A 18 03 24 00 01 51 05 07 01 A1 12 00 00 00 FF FF FF FF 7E 7E 7E
应答：  2A 18 03 24 00 01 51 05 07 01 A1 12 00 06 12 34 56 78 FF FF 08 FF FF FF FF 7E 7E 7E

*******************************************************************************/
void GetWhiteGroupName(void)
{
	uint32_t i,j;
	
	u8 SendEndbuf[8]={0x00,0xFF,0XFF,0XFF,0XFF,0x7e,0x7e,0x7e};    //结尾发送数据

	u8 xorbyte[1]={0x00};
	
	uint32_t  name_addr = 0;            //名单个数
	uint16_t  name_addr12 = 0;          //名单字节计数
	
	union     TwoByte   namecount={0};
	
	u8 data[72]={0X00};
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
	AT24CXX_2_Read(NUM_START, data,72);
	
	for(i=0;i<72;i=i+2)
	{
		namecount.i = (data[i] * 256 + data[i+1] + namecount.i);
	}
	namecount.i = namecount.i * (NAME_SIZE+2);
/*----------------------------------------------------------------------------*/
	                              
	communication_data[AGR_LEN_POS]    = namecount.c[1];     //名单字节数	
	communication_data[AGR_LEN_POS +1] = namecount.c[0];
			
  ReyurnCommand_Send(communication_data ,AGR_FIXED_SIZE) ;   //返回值发送 前面固定字节		
	
/*----------------------------------------------------------------------------*/
	for(i=0;i<36;i++)															//所有分组名单及公共组名单
	{							
		IWDG_Feed();//喂狗
		
		namecount.i = 0;
		
		namecount.i = (data[i*2] * 256 + data[i*2+1])*(NAME_SIZE+2);
		
		name_addr =GROUP_ST+GROUP_SPACE*i; 
		
		for(j=0;j<namecount.i;j++)
			{							
					communication_data[AGR_FIXED_SIZE + name_addr12++]=AT24CXX_2_ReadOneByte(name_addr);	
				   
					if(name_addr12>=1920)		
								{																		
									ReyurnCommand_Send(communication_data+AGR_FIXED_SIZE ,1920);    //返回值发送		
															 
									name_addr12=0;
									
									xorbyte[0] = (xorbyte[0]^GetXOR(communication_data + AGR_FIXED_SIZE, 1920));
									}			
		     name_addr++;
			 }			
	}
/*----------------------------------------------------------------------------*/
	if(name_addr12>0)
		{
		
		 ReyurnCommand_Send(communication_data+AGR_FIXED_SIZE ,name_addr12);    //返回值发送		
			 
		 xorbyte[0] = (xorbyte[0]^GetXOR(communication_data + AGR_FIXED_SIZE, name_addr12));	
		}
		
		SendEndbuf[0] = xorbyte[0];          //异或校验赋值
		ReyurnCommand_Send(SendEndbuf ,8);    //返回值发送			 
	
	  UARTSUCCESS();
		
}


/*******************************************************************************
清除白名单   OK

发送 :  2A 18 03 24 00 01 51 05 07 01 A1 13 00 00 00 FF FF FF FF 7E 7E 7E
应答 ： 2A 18 03 24 00 01 51 05 07 01 A1 13 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/
void ClearWhiteName(void)
{
	u8 i;
	
	for(i=0;i<72;i++)
	{
		AT24CXX_2_WriteOneByte(NUM_START+i,0);
	}	
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
	
}


/**********************************************************
//追加一条白名单   OK

发送 ： 2A 18 03 24 00 01 51 05 07 01 A1 14 00 06 12 34 56 78 FF FF 08 FF FF FF FF 7E 7E 7E
应答 ： 2A 18 03 24 00 01 51 05 07 01 A1 14 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
**********************************************************/

void AddOneWhiteGroupName()
{
	
	   if (AddOneCardno(communication_data + AGR_FIXED_SIZE) == 0)     //卡号存在
				{ 
				Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				}
				
	   else
				{
				Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
				}
}

/**********************************************************
//删除一条白名单    OK

发送 ： 2A 18 03 24 00 01 51 05 07 01 A1 15 00 06 12 34 56 78 FF FF 08 FF FF FF FF 7E 7E 7E
应答 ： 2A 18 03 24 00 01 51 05 07 01 A1 15 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
**********************************************************/

void DelOneWhiteGroupName()
{
	
	   if (DelOneCardno(communication_data + AGR_FIXED_SIZE)) 
				{ 					
					Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
				}
	   else
				{
					Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写	
				}
}


/**************************黑名单************************************/


/*******************************************************************************
下发黑名单

发送 ： 2A 18 03 24 00 01 51 05 07 01 A1 16 00 04 12 34 56 78 08 FF FF FF FF 7E 7E 7E
应答 ： 2A 18 03 24 00 01 51 05 07 01 A1 16 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/
void SetBackGroupName(void )
{	                    
	u16 i=0;
	
	u16 NAME_count=0;
	
	union TwoByte Group_addr={0};
	
	NAME_count = Data_length / NAME_SIZE;										//算出下发名单数量									
	
	for(i=0;i<NAME_count;i++)
	{		 	
		back_curgroup = GetSum(communication_data+ AGR_FIXED_SIZE+i*NAME_SIZE, NAME_SIZE) % BACK_GROUP_NUM;		//名单归属组号（散列因子）
		
		back_groupcount = Back_GetGroupCount(); 							//获取组名单数量
		/*--------------------------------------------------------------------*/
		if (back_groupcount < BACK_GROUP_RECORD) 							//分组未满存储
		{		
			Group_addr.i = Back_GetGroupAddress();									//组存储起始地址
		
			Group_addr.i =Group_addr.i+ (back_groupcount*NAME_SIZE);					//组存储地址接尾
			
			AT24CXX_Write(Group_addr.i,communication_data+AGR_FIXED_SIZE+i*NAME_SIZE,NAME_SIZE);			//写入名单
			
			back_groupcount+=1;														//名单+1
			
			Back_SaveGroupCount();													//保存组名单数量
		}
		/*--------------------------------------------------------------------*/
		else																 	//分组已满存储
		{ 
			back_commcount = Back_GetCommCount();											//读取公共组名单数量
			
			if(back_commcount < BACK_COMM_SIZE)											//公共区存储未满
			{
				Group_addr.i = BACK_COMM_ST;											//公共组存储起始地址
					
				Group_addr.i = Group_addr.i + (back_commcount*NAME_SIZE);			//公共组存储地址接尾
				
				AT24CXX_Write(Group_addr.i,communication_data+AGR_FIXED_SIZE+i*NAME_SIZE,NAME_SIZE);		//写入名单
				
				back_commcount+=1;													//名单+1
				
				Back_SaveCommCount();												//保存公共组组名单数量					
			}
			else																//公共区存储已满
		    {			  
				 
				 return;
			}
		}
		/*--------------------------------------------------------------------*/
	}
	
//	Back_SaveCommCount();												//保存公共组组名单数量
//	Back_SaveGroupCount();													//保存组名单数量
	
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}



/*******************************************************************************
名单黑读取：   OK

发送：  2A 18 03 24 00 01 51 05 07 01 A1 17 00 00 00 FF FF FF FF 7E 7E 7E
应答：  2A 18 03 24 00 01 51 05 07 01 A1 17 00 04 12 34 56 78 08 FF FF FF FF 7E 7E 7E
*******************************************************************************/
void GetBackGroupName(/*u16 List_NUM_START,u16 List_GROUP_ST,u8 Number*/)
		 {                     /*BACK_NUM_START        BACK_GROUP_ST            */    
			  
	u16 i,j;
	
	u8 SendEndbuf[8]={0x00,0xFF,0XFF,0XFF,0XFF,0x7e,0x7e,0x7e};    //结尾发送数据
			 
	u8 xorbyte[1]={0};
	
	uint32_t  name_addr = 0;
	
	u16  name_addr12 = 0;
	
	union TwoByte namecount={0};
	
	u8 data[64];
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
	AT24CXX_Read(BACK_NUM_START, data,64);
	
//	SendString(data, 64);	
	
	for(i=0;i<64;i=i+2)
	{
		namecount.i = (data[i] * 256 + data[i+1] + namecount.i);
	}
	namecount.i = namecount.i * NAME_SIZE;
/*----------------------------------------------------------------------------*/
	
	communication_data[AGR_LEN_POS]    = namecount.c[1];     //名单字节数	
	communication_data[AGR_LEN_POS +1] = namecount.c[0];
	
		 
	ReyurnCommand_Send(communication_data ,AGR_FIXED_SIZE) ;   //返回值发送	
	
/*----------------------------------------------------------------------------*/
	for(i=0;i<32;i++)															//所有分组名单及公共组名单
	{							
	IWDG_Feed();//喂狗
		
		namecount.i = 0;
		
		namecount.i = (data[i*2] * 256 + data[i*2+1])*(NAME_SIZE);
		
		name_addr =BACK_GROUP_ST+BACK_GROUP_SPACE*i; 
				
		
		for(j=0;j<namecount.i;j++)
			{							
					communication_data[AGR_FIXED_SIZE +name_addr12++]=AT24CXX_ReadOneByte(name_addr);	
    
					if(name_addr12>=1800)		
								{
																							
										 ReyurnCommand_Send(communication_data+AGR_FIXED_SIZE ,1800);    //返回值发送		
										 
										 name_addr12=0;
									
									   xorbyte[0] = (xorbyte[0]^GetXOR(communication_data + AGR_FIXED_SIZE, 1800));
									}			
		     name_addr++;
			 }			
	}
/*----------------------------------------------------------------------------*/
		
//	xorbyte[0] = (xorbyte[0]^GetXOR(communication_data + AGR_FIXED_SIZE, 8)); 
	
	if(name_addr12>0)
		{		
		 ReyurnCommand_Send(communication_data +AGR_FIXED_SIZE , name_addr12);    //回写	 
			 
		 xorbyte[0] = (xorbyte[0]^GetXOR(communication_data + AGR_FIXED_SIZE, name_addr12));
		}
	
		
		SendEndbuf[0] = xorbyte[0];          //异或校验赋值
		
		ReyurnCommand_Send(SendEndbuf , 8);    //回写	 
	
	  UARTSUCCESS();
		
}



/**********************************************************
//清除全部黑名单    OK

发送 ：  2A 18 03 24 00 01 51 05 07 01 A1 18 00 00 00 FF FF FF FF 7E 7E 7E
应答 ：  2A 18 03 24 00 01 51 05 07 01 A1 18 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

**********************************************************/
void ClearBackName(void)
{
	u8 i;
	
	for(i=0;i<64;i++)
	{
		AT24CXX_WriteOneByte(BACK_NUM_START+i,0);
	}	
	
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}	

/**********************************************************
//追加一条黑名单   OK

发送 ：  2A 18 03 24 00 01 51 05 07 01 A1 19 00 06 12 34 56 79 09 FF FF FF FF 7E 7E 7E
应答 ：  2A 18 03 24 00 01 51 05 07 01 A1 19 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
**********************************************************/

void AddOneBackGroupName()
{
 if (Back_AddOneCardno(communication_data + AGR_FIXED_SIZE) ==0) 
		{ 
		Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写	
		}
 else
		{
		 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
		}
}


/**********************************************************
//删除一条黑名单    OK

发送 ：  2A 18 03 24 00 01 51 05 07 01 A1 1A 00 06 12 34 56 79 09 FF FF FF FF 7E 7E 7E
应答 ：  2A 18 03 24 00 01 51 05 07 01 A1 1A 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
**********************************************************/

void DelOneBackGroupName()
{
	
	if (Back_DelOneCardno(communication_data + AGR_FIXED_SIZE)) 
		{ 
		 	 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
		}
	else
		{			
			Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
		}
}


/*************************************** 打卡记录 ***********************************************************/

/*******************************************************************************
读取打卡记录数量      OK

发送 ：  2A 18 03 24 00 01 51 05 07 01 A1 1B 00 00 00 FF FF FF FF 7E 7E 7E
应答 ：  2A 18 03 24 00 01 51 05 07 01 A1 1B 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/
void GetMaxRecordCount(void)
{
	union FourByte maxrecordnum ={0};     //最大记录容量

	union FourByte allnum ={0};           //实际存储数量
	
	union FourByte Actualnum ={0};	       //实际刷卡数量

	union FourByte Err_Cardnum ={0};	     //错误刷卡数量
	
	union FourByte Operationnum ={0};	   //命令操作数量	
	
	u8 RecordNUMbuf[20]={0,0,0,0};   //名单计数数量 前4字节可存储最大数量  后4字节已经存储名单数量
	
	maxrecordnum.i= MAX_RECORD;      //存储最大记录数量

	allnum.i         = recordcount;     //当前存储的记录数
	
	Actualnum.i      = Actual_Card_number;   //实际刷卡数量
	
	Err_Cardnum.i    = Err_Card_number;     //错误刷卡数量
	
	Operationnum.i   = Operation_Comm_number;     //命令操作数量	
		
	
  turn_4_data(RecordNUMbuf ,   maxrecordnum.c);
	
	turn_4_data(RecordNUMbuf+4 , allnum.c);
	
	turn_4_data(RecordNUMbuf+8 , Actualnum.c);	
	
	turn_4_data(RecordNUMbuf+12 ,Err_Cardnum.c);	
	
	turn_4_data(RecordNUMbuf+16 ,Operationnum.c);
	
	Send_ReyurnGETCommand_Value(RecordNUMbuf , 20);    //回写

	}


/*******************************************************************************
按地址采集打卡记录    OK
	
发送 ：  2A 18 03 24 00 01 51 05 07 01 A1 1C 00 08 00 00 00 00 01 00 00 00 01 FF FF FF FF 7E 7E 7E
应答 ：  2A 18 03 24 00 01 51 05 07 01 A1 1C 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/
void GetRecord_Address(void)
{
	union  FourByte   startaddress={0};     //读取的名单数量
	union  FourByte   getcount={0};         //读取名单的起始地址
	uint32_t  packagenum=0;                 //包数量
	uint8_t   packageSurplusnum=0;          //包剩余数量	
	uint16_t i=0;
//	uint16_t bitmun =0;
	union  TwoByte   package_cycle={0};         //包编号
	union  TwoByte   bitmun={0};         //数量	
	
	turn_4_data(startaddress.c,communication_data + AGR_FIXED_SIZE);          //拷贝起始地址
	turn_4_data(getcount.c,    communication_data + AGR_FIXED_SIZE+4);        //拷贝需要获取名单数量
	
//	MyStrCPY(startaddress.c,    communication_data + AGR_FIXED_SIZE,     4);       //拷贝起始地址
//	MyStrCPY(getcount.c,        communication_data + AGR_FIXED_SIZE + 4, 4);       //拷贝需要获取名单数量
	
	packagenum         =  getcount.i /64 ;
	packageSurplusnum  =  getcount.i %64 ;
	

//  printf("%d\r\n",packagenum);
//	printf("%d\r\n",packageSurplusnum);
	
		for(i=0;i<packagenum;i++)
			{	
				IWDG_Feed();//喂狗
				
				kt_beat_info.count=0;     //心跳计数置0
				
        package_cycle.i = i+1;
				
				communication_data[AGR_LEN_POS]   = 0X07;                  //数据长度		
				communication_data[AGR_LEN_POS2]  = 0X82 ;                 //数据长度
																	
				communication_data[AGR_LEN_POS2+1]   = package_cycle.c[1];                  //拷贝包编号	
				communication_data[AGR_LEN_POS2+2]   = package_cycle.c[0] ;                 //拷贝包编号
				
				SPI_Flash_Read(communication_data + AGR_FIXED_SIZE +2 ,startaddress.i + W25X_PAGE_SIZE *i, W25X_PAGE_SIZE ) ;  //拷贝数据  从Flash中读取
											
				communication_data[AGR_ROX_POS + W25X_PAGE_SIZE +1] = GetXOR(communication_data + AGR_DATA_POS, W25X_PAGE_SIZE +2);//xor检验位
							
				MyStrCPY(communication_data + AGR_RETAIN_POS +W25X_PAGE_SIZE +1, Retainbitbuf, 4);//保留字节
				
				MyStrCPY(communication_data +AGR_END_POS + W25X_PAGE_SIZE +2, Return_Endvalue, AGR_END_SIZE);//码尾
				
				ReyurnCommand_Send(communication_data ,AGR_MAXEND_POS +W25X_PAGE_SIZE +2 ) ;   //返回值发送		
								
			}	

     if(packageSurplusnum) 
			{
			  package_cycle.i = i +1;
				
				bitmun.i = packageSurplusnum*30  ;
			
				bitmun.i = bitmun.i +2;
				
				communication_data[AGR_LEN_POS]   = bitmun.c[1];                  //数据长度		
				communication_data[AGR_LEN_POS2]  = bitmun.c[0] ;                 //数据长度							
				
				bitmun.i = bitmun.i -2;
				
				communication_data[AGR_LEN_POS2+1]   = package_cycle.c[1];                  //拷贝包编号	
				communication_data[AGR_LEN_POS2+2]   = package_cycle.c[0] ;                 //拷贝包编号
				
				SPI_Flash_Read(communication_data + AGR_FIXED_SIZE +2 ,startaddress.i + packagenum * W25X_PAGE_SIZE, bitmun.i) ;  //拷贝数据  从Flash中读取
			
				communication_data[AGR_ROX_POS + bitmun.i +1] = GetXOR(communication_data + AGR_DATA_POS, bitmun.i +2);//xor检验位
						
				MyStrCPY(communication_data + AGR_RETAIN_POS +bitmun.i +1, Retainbitbuf, 4);//保留字节
				
				MyStrCPY(communication_data +AGR_END_POS + bitmun.i +2, Return_Endvalue, AGR_END_SIZE);//码尾
				
				ReyurnCommand_Send(communication_data , AGR_MAXEND_POS + bitmun.i +2);	
						
			}
		UARTSUCCESS();	
			
}


/*******************************************************************************
采集记录     OK

发送:  2A 18 03 24 00 01 51 05 07 01 A1 1D 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 51 05 07 01 00 23 00 07 17 07 10 17 40 35 31 7E 7E 7E
*******************************************************************************/

void Get_MaxRecord(void)
{
	uint16_t i;

	union  TwoByte   package_cycle={0};         //包编号
	
	//i= 4369  1920
	
	for(i=0;i<W25X_PAGE_NUM;i++)
		{
		IWDG_Feed();//喂狗
			
		kt_beat_info.count=0;	     //心跳计数置0
			
		package_cycle.i = i+1;
		
		communication_data[AGR_LEN_POS]   = 0X07;                  //数据长度		
		communication_data[AGR_LEN_POS2]  = 0X82 ;                 //数据长度
															
		communication_data[AGR_LEN_POS2+1]   = package_cycle.c[1];                  //拷贝包编号	
		communication_data[AGR_LEN_POS2+2]   = package_cycle.c[0] ;                 //拷贝包编号
		
		W25X_Read_Page(communication_data + AGR_FIXED_SIZE +2 ,i);         			//拷贝数据  从Flash中读取	
									
		communication_data[AGR_ROX_POS + W25X_PAGE_SIZE +1] = GetXOR(communication_data + AGR_DATA_POS, W25X_PAGE_SIZE +2);//xor检验位
					
		MyStrCPY(communication_data + AGR_RETAIN_POS +W25X_PAGE_SIZE +1, Retainbitbuf, 4);//保留字节
		
		MyStrCPY(communication_data +AGR_END_POS + W25X_PAGE_SIZE +2, Return_Endvalue, AGR_END_SIZE);//码尾
		
		ReyurnCommand_Send(communication_data ,AGR_MAXEND_POS +W25X_PAGE_SIZE +2 ) ;   //返回值发送		
				
		}
	UARTSUCCESS();
}


/*******************************************************************************
清除打卡记录    OK

发送 ：  2A 18 03 24 00 01 51 05 07 01 A1 1E 00 00 00 FF FF FF FF 7E 7E 7E
应答 ：  2A 18 03 24 00 01 51 05 07 01 A1 1E 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/
void ClearRecord(void)
{

	u8 DEF_MACHNO[] = {0x00,0x00,0x00,0x00};
        
	recordcount = 0;
	
	Flow_number = 0;
	
  Actual_Card_number =0;                //实际数卡数量
 
  Err_Card_number =0;                   //错误数卡数量
 
  Operation_Comm_number =0;             //操作命令数量	
	
	maxrecordcount_err =0;                  //超出最大容量
	
	SaveRecordcount_Err();  //保存存储错误
	
	AT24CXX_Write( RECORD_POS,          DEF_MACHNO,   4);		// 初始化数据采集起始地址
	
	AT24CXX_Write( FLOW_NUMBER_POS,     DEF_MACHNO,   4);		// 初始化数据采集起始地址
	
	AT24CXX_Write( ACTUAL_CARD_POS  ,   DEF_MACHNO ,  4);   //初始化实际数卡数量
	
	AT24CXX_Write( ERR_CARD_POS  ,      DEF_MACHNO ,  4);   //初始错误数卡数量
	
	AT24CXX_Write( OPERATION_COMM_POS  ,DEF_MACHNO ,  4);  //初始操作命令数量
			
  DeleteConsumption_All_Money(); //清除消费总额
		
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写	
	
}  



/*******************************************************************************
设置刷卡间隔延时  
发送:  2A 18 03 24 00 01 51 05 07 01 A1 40 00 01 01 01 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 24 00 01 51 05 07 01 A1 40 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/

void Set_Interval_delay(void)
{

	if (communication_data[AGR_LEN_POS2] != 0X01) 
	{
		Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写	
		
		return;
	}
	
  AT24CXX_Write(INTERVAL_IDX, communication_data + AGR_FIXED_SIZE , 1);
	
	AT24CXX_Read(INTERVAL_IDX, interval_time , 1);   //刷卡延时
		
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}




/*******************************************************************************
设置是否需要读写IC卡扇区数据：   OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 41 00 01 01 01 FF FF FF FF 7E 7E 7E   （读写）
发送：2A 18 03 26 00 01 51 05 07 01 A1 41 00 01 00 00 FF FF FF FF 7E 7E 7E    （不读写）

应答：2A 18 03 26 00 01 51 05 07 01 A1 41 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/

void Set_ReadorWriteICData_Mode(void)
		{
			if (communication_data[AGR_LEN_POS2] != 0x01) //检验数据长度 
	  	{
				Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
				return;
			}
			
	AT24CXX_Write(OUT_CALL_IDX,communication_data+AGR_FIXED_SIZE , 1);	
				
  InitSysvar();            //初始化系统参数、扩展参数 
			
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
		
		}


/*******************************************************************************
设置是否检查IC卡密码：     OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 42 00 01 01 01 FF FF FF FF 7E 7E 7E   （检测）
发送：2A 18 03 26 00 01 51 05 07 01 A1 42 00 01 00 00 FF FF FF FF 7E 7E 7E    （不检测）

应答：2A 18 03 26 00 01 51 05 07 01 A1 42 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/


void Set_Check_Sector_Password_Mode(void)
		{
			if (communication_data[AGR_LEN_POS2] != 0x01) //检验数据长度 
	  	{
				Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				return;
			}	
	AT24CXX_Write(CHECK_PW_IDX,communication_data+AGR_FIXED_SIZE , 1);		
							
	InitSysvar();            //初始化系统参数、扩展参数 
			
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
		
		}

		
	/*******************************************************************************
获取扩展参数：   OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 43 00 01 01 01 FF FF FF FF 7E 7E 7E  

应答：2A 18 03 26 00 01 51 05 07 01 A1 43 00 05 01 01 FF FF FF FF FF FF FF FF 7E 7E 7E

*******************************************************************************/	

void Get_Extvar(void)
{
  u8 extvarbuff[EXTVAR_SIZE] ={0};
	
	AT24CXX_Read(EXTVAR_ST, extvarbuff , EXTVAR_SIZE );
	
  Send_ReyurnGETCommand_Value(extvarbuff , EXTVAR_SIZE);    //回写	

}


	/*******************************************************************************
设置门禁时段：   OK

发送： 2A 18 03 26 00 01 51 05 07 01 A1 44 00 20 00 00 04 00 04 01 08 00 08 01 14 00 14 01 18 00 00 00 04 00 04 01 08 00 08 01 14 00 04 01 08 00 00 FF FF FF FF 7E 7E 7E 
应答： 2A 18 03 26 00 01 51 05 07 01 A1 44 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/	

void Set_Permission_Period(void)
{
	if (communication_data[AGR_LEN_POS2] != 0x20) //检验数据长度 
	  	{
		  Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
			return;
			}
			
	AT24CXX_Write(PERMISSION_PERIOD_POS,communication_data +AGR_FIXED_SIZE,32);	              //24C512写门禁时段
	
	InitTime_Interval_Detection();                            //门禁时段初始化		
			
  Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}


	/*******************************************************************************
读取门禁时段：    OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 45 00 00 00 FF FF FF FF 7E 7E 7E
应答：2A 18 03 26 00 01 51 05 07 01 A1 45 00 20 00 00 04 00 04 01 08 00 08 01 14 00 14 01 18 00 00 00 04 00 04 01 08 00 08 01 14 00 04 01 08 00 00 FF FF FF FF 7E 7E 7E

*******************************************************************************/	

void Get_Permission_Period(void)
{   
 	
	Send_ReyurnGETCommand_Value(Permission_Period , 32);    //回写

}



	/*******************************************************************************
通道时段使能：        OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 46 00 06 45 6E 61 62 6C 65 21 FF FF FF FF 7E 7E 7E

应答：2A 18 03 26 00 01 51 05 07 01 A1 46 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/	

void Set_Passage_Time_Enable(void)
{
	if (communication_data[AGR_LEN_POS2] != 0x06) //检验数据长度 
	  	{
		  Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
			return;
			}
			
	AT24CXX_Write(PASSAGE_TIME_ENABLE_POS,communication_data+AGR_FIXED_SIZE,6);	              //24C512写门禁时段
	
	InitPassage_TimeEnable();                                 //通道使能初始化		
			
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}


	/*******************************************************************************
设置通道时间段：          OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 47 00 20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF FF 7E 7E 7E

应答：2A 18 03 26 00 01 51 05 07 01 A1 47 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/	

void Set_Passage_Time(void)
{
	if (communication_data[AGR_LEN_POS2] != 0x20) //检验数据长度 
	  	{
		  Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
			return;
			}
			
	AT24CXX_Write(PASSAGE_TIME_POS,communication_data+AGR_FIXED_SIZE,32);	              //24C512写通道时间段
	
	InitPassage_Time();                            //通道时间段初始化		
			
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}

	/*******************************************************************************
读取通道时间段：     OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 48 00 00 00 FF FF FF FF 7E 7E 7E

应答：2A 18 03 26 00 01 51 05 07 01 A1 48 00 20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_Passage_Time(void)
{ 	  
	
  Send_ReyurnGETCommand_Value(Passage_Timebuff , 32);    //回写

}


	/*******************************************************************************
设置节假日日期：    OK

发送：2A 18 03 26 00 01 51 05 07 01 A1 49 00 25 12 31 01 08 02 01 02 03 02 04 02 08 02 13 02 14 03 12 03 29 02 27 03 28 03 01 03 05 03 07 03 10 02 19 02 26 60 5A 7E 7E 7E

应答：2A 18 03 26 00 01 51 05 07 01 A1 49 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E

*******************************************************************************/	

void Set_Holiday_Vacations(void)
{
	if (communication_data[AGR_LEN_POS2] != 0x25) //检验数据长度 
	  	{
		  Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
				
			return;
			}
			
	AT24CXX_Write(HOLIDAY_VACATIONS_POS,communication_data+AGR_FIXED_SIZE,37);	              //24C512写节假日日期
	
	InitHoliday_Vacations();                            //节假日日期初始化		
			
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写

}



	/*******************************************************************************
读取节假日日期：    OK

发送： 2A 18 03 26 00 01 51 05 07 01 A1 4A 00 00 00 FF FF FF FF 7E 7E 7E
应答： 2A 18 03 26 00 01 51 05 07 01 A1 4A 00 25 12 31 01 08 02 01 02 03 02 04 02 08 02 13 02 14 03 12 03 29 02 27 03 28 03 01 03 05 03 07 03 10 02 19 02 26 60 5A FF FF FF FF 7E 7E 7E

*******************************************************************************/	

void Get_Holiday_Vacations(void)
{	  
	Send_ReyurnGETCommand_Value(Holiday_Vacationsbuff , 37);    //回写

}


void TEST_GQ(void)
{
	u8 temp1[7]={0X00};

	
//	AT24CXX_Read(NEWDATE_ADDRST, temp1 , 4); 										//读取flash采集始地址
//		SendString(temp1,4);
	
	AT24CXX_Read(SECTION_POS, temp1 , 7); 				//读取flash采集始地址

	//send(SOCK_TCPC,temp1,7);	
	
		
}



	/*******************************************************************************
设定当次最大限额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 61 00 03 01 00 01 02 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 61 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_Sub_Maximum_Limit(void)
{	  
	
	 MyStrCPY(Sub_Maximum_Limitbuf,communication_data + AGR_FIXED_SIZE, 4);    //拷贝数据  当次最大限额
	
	
	 SaveSub_Maximum_Limit();    //保存当次消费限额
	
//	 InitSub_Maximum_Limit();   //初始化当次消费限额
	
	 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}

	/*******************************************************************************
读取当次最大限额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 62 00 00 00 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 62 00 03 01 01 00 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_Sub_Maximum_Limit(void)
{	  
	
	 Send_ReyurnGETCommand_Value(Sub_Maximum_Limitbuf , 4);    //回写
}


	/*******************************************************************************
设定消费模式

发送： 2A 18 03 26 00 01 51 05 07 01 A1 63 00 01 01 01 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 63 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_Machine_Category(void)
{	  
	
	
	 MyStrCPY(Machine_category,communication_data + AGR_FIXED_SIZE, 1);    //拷贝数据  
	
	 SaveMachine_Category();   //初始化消费模式
	
	 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}

	/*******************************************************************************
读取消费模式

发送： 2A 18 03 26 00 01 51 05 07 01 A1 64 00 00 00 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 64 00 01 01 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_Machine_Category(void)
{	  
	
	 Send_ReyurnGETCommand_Value(Machine_category , 1);    //回写
}


	/*******************************************************************************
设定就餐时段

发送： 2A 18 03 26 00 01 51 05 07 01 A1 65 00 10 00 00 23 59 00 00 23 59 00 00 23 59 00 00 23 59 01 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 65 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_Fixed_consumption_Timer(void)
{	  	
	 MyStrCPY(Fixed_consum_Timerbuf,communication_data + AGR_FIXED_SIZE, 16);    //拷贝数据  定值扣费时间段
	
	 InitFixed_consumption_Timer();                  //初始化定值扣费时间段
	
	 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}

	/*******************************************************************************
读取就餐时段

发送： 2A 18 03 26 00 01 51 05 07 01 A1 67 00 00 00 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 67 00 20 00 00 23 59 00 00 23 59 00 00 23 59 00 00 23 59 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_Fixed_consumption_Timer(void)
{	  
	
	 Send_ReyurnGETCommand_Value(Fixed_consum_Timerbuf , 32);    //回写
}



	/*******************************************************************************
设定定值扣费金额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 68 00 08 00 00 05 00 00 00 05 00 01 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 68 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_Fixed_consumption_Money(void)
{	  
	
	
	 MyStrCPY(Fixed_consum_Moneybuf,communication_data + AGR_FIXED_SIZE, 8);    //拷贝数据  定值扣费时间段
	
	 InitFixed_consumption_Money();                  //初始化定值扣费金额
	
	 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}

	/*******************************************************************************
读取定值扣费金额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 6A 00 00 00 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 6A 00 10 00 00 23 59 00 00 23 59 00 00 23 59 00 00 23 59 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_Fixed_consumption_Money(void)
{	  
	
	 Send_ReyurnGETCommand_Value(Fixed_consum_Moneybuf , 16);    //回写
}



	/*******************************************************************************
设定菜单扣费金额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 6B 00 08 00 00 05 00 00 00 05 00 01 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 6B 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_Carte_Consum_Money(void)
{	  
	
	
	 MyStrCPY(Carte_consum_Moneybuf,communication_data + AGR_FIXED_SIZE, 18);    //拷贝数据  定值扣费时间段
	
	 InitCarte_consum_Money();                  //初始化菜单扣费金额
	
	 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
}

	/*******************************************************************************
查询余额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 6D 00 06 01 02 03 04 05 06 00 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_Carte_Consum_Money(void)
{	  
	
	 Send_ReyurnGETCommand_Value(Carte_consum_Moneybuf , 18);    //回写
}


	/*******************************************************************************
返回查询的余额


应答： 2A 18 03 26 00 01 51 05 07 01 A1 6E 00 10 00 00 23 59 00 00 23 59 00 00 23 59 00 00 23 59 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Query_Balance_Return(void)
{	  
		uint8_t    Namebuff[10]    ={0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20};    // 姓名
		uint8_t    US_Balance_Show[10]   ={0X0B,0X0B,0X0B,0X0B,0X0B,0X0B,0X0B,0X0B,0x00,0x00};    // 余额	
		uint8_t    US_Balance[8]   ={0X0B,0X0B,0X0B,0X0B,0X0B,0X0B,0X0B,0X0B};    // 余额
		uint8_t    UserBalancebuf[4];
		uint8_t    t = 0;
		
		ClrScreen();  //清屏	
	
		SetBG_Color(LCD_DARK);
		
		SetDisplayInvert(0);
		
		SetFontType((LCD_ASC16_FONT<<4)|LCD_HZK32_FONT);
		
		SetFG_Color(LCD_WHITE);		
	
		Clear_Standby_Display_Time(5);
	
	 MyStrCPY(UserBalancebuf ,communication_data + AGR_FIXED_SIZE +12, 4);    //拷贝数据 余额

	 MyStrCPY(Namebuff    ,communication_data + AGR_FIXED_SIZE +18, 10);    //拷贝数据  姓名
		
/********************** 处理余额数据 ************************************/		
	
	US_Balance[0]= (UserBalancebuf[0]>>4)&0X0F;
	US_Balance[1]= (UserBalancebuf[0]>>0)&0X0F;
	
	US_Balance[2]= (UserBalancebuf[1]>>4)&0X0F;
	US_Balance[3]= (UserBalancebuf[1]>>0)&0X0F;
	
	US_Balance[4]= (UserBalancebuf[2]>>4)&0X0F;
	US_Balance[5]= (UserBalancebuf[2]>>0)&0X0F;
	
	US_Balance[6]= (UserBalancebuf[3]>>4)&0X0F;
	US_Balance[7]= (UserBalancebuf[3]>>0)&0X0F;
	
	for(t =0;t< 5 ;t++)
	{
	   if( US_Balance[t] ==0X00)
		 {
		   US_Balance[t] = 10; 
		 }	 
		 else
		 {
		  break;
		 } 
	}		
		
	US_Balance_Show[0]      = num_lcd[US_Balance[0]];
	US_Balance_Show[1]      = num_lcd[US_Balance[1]];
	US_Balance_Show[2]      = num_lcd[US_Balance[2]];
	US_Balance_Show[3]      = num_lcd[US_Balance[3]];
	US_Balance_Show[4]      = num_lcd[US_Balance[4]];															
	US_Balance_Show[5]      = num_lcd[US_Balance[5]];
	US_Balance_Show[6]      = num_lcd[12];					
	US_Balance_Show[7]      = num_lcd[US_Balance[6]];					
	US_Balance_Show[8]      = num_lcd[US_Balance[7]];
		
	if(communication_data[AGR_FIXED_SIZE + 28] ==1)      	
	 {
		 if(t==0) t=1;
		 
	 	US_Balance_Show[t-1]      = num_lcd[13];
	 }	
	
	
	PutString (25,50,"用    户:");	
	PutString (180,50,Namebuff);		
	
	PutString (25,100,"余    额:");

	PutString (180  ,100,US_Balance_Show);	

	PutString (350,100,"元");		
		
}


/*******************************************************************************
  读取消费总额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 73 00 06 01 02 03 04 05 06 00 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_Total_Consumption(void)
{	  
	
	 Send_ReyurnGETCommand_Value(Consum_Allmoney.c , 4);    //回写
}

/*******************************************************************************
  清除消费总额

发送： 2A 18 03 26 00 01 51 05 07 01 A1 74 00 06 01 02 03 04 05 06 06 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_Clean_Total_Consumption(void)
{	  
	if (communication_data[AGR_LEN_POS2] != 0x06) //检验数据长度 
		{
			Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
							
			return;
		}
	
	DeleteConsumption_All_Money();   //清除消费总额  
	
	Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
		
//	 Send_ReyurnGETCommand_Value(Consum_Allmoney.c , 4);    //回写
}



	/*******************************************************************************
读取GPS信息

发送： 2A 18 03 26 00 01 51 05 07 01 A1 F5 00 00 00 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 F5 00 10 00 00 23 59 00 00 23 59 00 00 23 59 00 00 23 59 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Get_GPS_Information(void)
{	  
	
	
	
	
	 Send_ReyurnGETCommand_Value(EC20GPSBUF , EC20GPSLEN);    //回写
}


	/*******************************************************************************
自动上报GPS信息

发送： 2A 18 03 26 00 01 51 05 07 01 A1 F6 00 00 00 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 F6 00 10 00 00 23 59 00 00 23 59 00 00 23 59 00 00 23 59 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void AutoUP_GPS_Information(void)
{	  
	uint8_t  AutoUP_GPSbuf[6]={0x18,0x09,0x29};
	
	
	
	communication_data[0]   = 0X2A;         //头	
	MyStrCPY(communication_data + AGR_CODE_POS, AutoUP_GPSbuf, 5);         //流水号
	MyStrCPY(communication_data + AGR_MACH_POS, machno, MACH_SIZE); //机号
	
	communication_data [10] = 0XA4; //通讯类型	
	communication_data [11] = 0XF6; //命令		
	
	communication_data[AGR_LEN_POS]   = (EC20GPSLEN>>8)&0XFF;         //数据长度		
	communication_data[AGR_LEN_POS+1] = EC20GPSLEN&0XFF;       //数据长度;       //数据长度
	
	MyStrCPY(communication_data + AGR_DATA_POS, EC20GPSBUF, EC20GPSLEN);//数据
		  
	communication_data[AGR_ROX_POS + EC20GPSLEN -1] = GetXOR(communication_data + AGR_DATA_POS, EC20GPSLEN);//xor检验位
				
  MyStrCPY(communication_data + AGR_RETAIN_POS +EC20GPSLEN -1, Retainbitbuf, 4);//保留字节
	
	MyStrCPY(communication_data +AGR_END_POS + EC20GPSLEN, Return_Endvalue, AGR_END_SIZE);//码尾
	
  EC20_SendString(communication_data , AGR_MAXEND_POS + EC20GPSLEN);
	
	// Send_ReyurnGETCommand_Value(Carte_consum_Moneybuf , 18);    //回写
}



/*******************************************************************************
查询IMSI   
发送:  2A 18 03 24 00 01 51 05 07 01 A1 Fa 00 00 00 FF FF FF FF 7E 7E 7E  
应答： 2A 18 03 26 00 01 51 05 07 01 A1 Fa 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E
*******************************************************************************/
void Get_IMEI_IMSI_Machno(void )
{
  uint8_t  Getbuf[40]={0x00};
	memcpy(Getbuf     , gl_imei, 15);	
	memcpy(Getbuf +15 , gl_imsi, 15);		
	memcpy(Getbuf +30 , machno, 4);	
	
	Send_ReyurnGETCommand_Value(Getbuf , 34);    //回写
	
}


	/*******************************************************************************
开始固件更新

发送： 2A 18 03 26 00 01 51 05 07 01 A1 A0 00 06 01 02 03 04 AA BB 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 A0 00 02 4F 4B 04 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_IAP_Start_Update(void)
{	  
	
		union  EightByte  comic8 ={0};
		
		comic8.i = False_Random_number();
	

	 if(Check_IAP_Start_Updata(communication_data+AGR_FIXED_SIZE  , 6)) 
	 {	 	 
	 	 Send_ReyurnGETCommand_Value(comic8.c , 4);    //回写	 
		 	 
	 }
	 else
	 {
	   Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
	 }
	
}

	/*******************************************************************************
发送固件数据包

发送： 2A 18 03 26 00 01 51 05 07 01 A1 A1 00 FF 01 02 XX XX XX XX AA AA 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 A1 00 02 01 02 01 FF FF FF FF 7E 7E 7E


*******************************************************************************/	

void Set_Handle_IAP_Data(void)
{	  
	uint8_t  databuf[4]={0x00};
	
	MyStrCPY(databuf, communication_data+AGR_FIXED_SIZE, 2);    //拷贝包编号
	
	 if(Handle_IAP_Data(communication_data+AGR_FIXED_SIZE +2 , Data_length-2))    //处理固件包内容
	 {
	 	 Send_ReyurnGETCommand_Value(databuf , 2);    //回写包编号
	 }
	 else
	 {
	   Send_ReyurnGETCommand_Value(Return_Codebuf + RETURN_ERR_POS , RETURN_ERR_SIZE);    //错误码回写
	 }
	 
}




	/*******************************************************************************
固件升级结束

发送： 2A 18 03 26 00 01 51 05 07 01 A1 A2 00 00 00 FF FF FF FF 7E 7E 7E

应答： 2A 18 03 26 00 01 51 05 07 01 A1 A2 00 03 XX 01 02 01 FF FF FF FF 7E 7E 7E

*******************************************************************************/	

void Set_Check_IAP_END_Updata(void)
{	  
	 uint8_t  databuf[4]={0x00};
	 union    TwoByte  ic2 = {0};
	 
	 
		ic2.i = iapreceivepackcnt ;
		 
		databuf[1]  = ic2.c[0] ;
		databuf[2]  = ic2.c[1] ;
		 	 
	 
	 if(Check_IAP_END_Updata())    //处理固件包内容
	 {
		databuf[0] = 0x01;
	
	 }
	 else 
	 {
	  databuf[0] = 0x00;
	 }
	 
	 Send_ReyurnGETCommand_Value(databuf , 3);    //回写校验结果 
}





/*******************************************************************************
读取固件内容
	
发送 ： 2A 18 03 26 00 01 51 05 07 01 A1 A3 00 00 00 FF FF FF FF 7E 7E 7E
应答 ： 2A 18 03 26 00 01 51 05 07 01 A1 A3 00 FF 01 02 XX XX XX XX 01 FF FF FF FF 7E 7E 7E

*******************************************************************************/
void Get_IAP_Read_Data(void)
{
	
	
	
	uint16_t   packagenum=0;                 //包数量
	uint16_t   packageSurplusnum=0;          //包剩余数量	
	uint16_t   i=0;

	union  TwoByte   package_cycle={0};         //包编号

	packagenum         =  iapallcnt /(IAP_SIZE/2) ;     //计算多少个包
	
	packageSurplusnum  =  iapallcnt %(IAP_SIZE/2) ;     //计算不够1包剩余的数量
	
//  printf("%d\r\n",packagenum);
//	printf("%d\r\n",packageSurplusnum);
	
	
		for(i=0;i<packagenum;i++)
			{	
				IWDG_Feed();//喂狗
				
				kt_beat_info.count=0;     //心跳计数置0
				
        package_cycle.i = i+1;    //包编号
				
				communication_data[AGR_LEN_POS]   = (((IAP_SIZE/2)+2)>>8)&0XFF;         //数据长度		
				communication_data[AGR_LEN_POS+1] = ((IAP_SIZE/2)+2)&0XFF;              //数据长度;      
																	
				communication_data[AGR_LEN_POS2+1]   = package_cycle.c[0];                  //拷贝包编号	
				communication_data[AGR_LEN_POS2+2]   = package_cycle.c[1] ;                 //拷贝包编号
					
			  STMFLASH_Read(FLASH_APP2_ADDR + i*IAP_SIZE/2 , (u16*)(communication_data + AGR_FIXED_SIZE +2) , IAP_SIZE/4);      // 拷贝数据  从STM32内部Flash中读取
				
				communication_data[AGR_ROX_POS + IAP_SIZE/2 +1] = GetXOR(communication_data + AGR_DATA_POS, IAP_SIZE/2 +2);//xor检验位
							
				MyStrCPY(communication_data + AGR_RETAIN_POS +IAP_SIZE/2 +1, Retainbitbuf, 4);//保留字节
				
				MyStrCPY(communication_data +AGR_END_POS + IAP_SIZE/2 +2, Return_Endvalue, AGR_END_SIZE);//码尾
				
				ReyurnCommand_Send(communication_data ,AGR_MAXEND_POS +IAP_SIZE/2 +2 ) ;   //返回值发送		
							

			}	

     if(packageSurplusnum) 
			{
			  package_cycle.i = i +1;
						
				communication_data[AGR_LEN_POS]   = ((packageSurplusnum+2)>>8)&0XFF;         //数据长度		
				communication_data[AGR_LEN_POS+1] =  (packageSurplusnum+2)&0XFF;              //数据长度;  						
		
				communication_data[AGR_LEN_POS2+1]   = package_cycle.c[0];                  //拷贝包编号	
				communication_data[AGR_LEN_POS2+2]   = package_cycle.c[1] ;                 //拷贝包编号
								
				STMFLASH_Read(FLASH_APP2_ADDR + packagenum*(IAP_SIZE/2) , (u16*)(communication_data + AGR_FIXED_SIZE +2) , packageSurplusnum/2);      // 拷贝数据  从STM32内部Flash中读取			
			
				communication_data[AGR_ROX_POS + packageSurplusnum +1] = GetXOR(communication_data + AGR_DATA_POS, packageSurplusnum +2);//xor检验位
						
				MyStrCPY(communication_data + AGR_RETAIN_POS +packageSurplusnum +1, Retainbitbuf, 4);//保留字节
				
				MyStrCPY(communication_data +AGR_END_POS + packageSurplusnum +2, Return_Endvalue, AGR_END_SIZE);//码尾
				
				ReyurnCommand_Send(communication_data , AGR_MAXEND_POS + packageSurplusnum +2);	
						
			}
		UARTSUCCESS();	
			
}


/*******************************************************************************
固件升级确认完成
	
发送 ： 2A 18 03 26 00 01 51 05 07 01 A1 A4 00 02 4F 4B 01 FF FF FF FF 7E 7E 7E
应答 ： 2A 18 03 26 00 01 51 05 07 01 A1 A4 00 02 4F 4B 01 FF FF FF FF 7E 7E 7E

*******************************************************************************/
void Set_IAP_Updata_OK(void)
{
		 Send_ReyurnGETCommand_Value(Return_Codebuf , RETURN_SUCCESS_SIZE);    //回写
	
     IAP_OK_System_Boot();
	
	   SoftReset();   //设备复位
	
}

/**********************************************************
//处理485通讯
**********************************************************/
void ProcessRS485(void)
{	
	u8   DEF_MACHNO[] = {0X51,0X05,0X07,0X01};
  u8	 ch;
	
	if (endcount < 3) return;
	
	endcount = 0;
		
	if (comidx < 10) { goto com_rst; }	

	if (combuff[0] != '*') { goto com_end ; }  

	if (!MyStrEQ(combuff + AGR_MACH_POS, machno, 4) && 
		!MyStrEQ(combuff + AGR_MACH_POS, DEF_MACHNO, 4)) { goto com_end ; }
		
	Data_length=separate_value_2_1(combuff+AGR_LEN_POS);         	//2字节地址合并
		
	ch = GetXOR(combuff + AGR_DATA_POS, Data_length );          
		
	if (ch != combuff[Data_length + AGR_DATA_POS]) { goto com_end ; }//异或检验
				
	if (combuff[AGR_TYPE_POS]==0XA1)
	{
     memcpy(communication_data,combuff,sizeof(combuff));	
		
		 Process_com_data();
	
	//	 InitCombuff();
		}
		
	com_end:
	  InitCombuff();

  com_rst:
	  inPCOM = 0;

}




/**********************************************************
//初始化串口通讯缓冲区
**********************************************************/
void InitCombuff(void)
{
	comidx = 0;	  // 串口数据个数
	endcount = 0;	//结束标志~	 

	
	memset(combuff,0,sizeof(combuff));
}



/**********************************************************
//CMD命令处理函数
**********************************************************/

void Process_com_data(void)
		{		
		  	uint8_t  cmdbuf[1]={0x00};
			
			 cmdbuf[0]=communication_data[AGR_CMD_POS];
			
			
		switch (communication_data[AGR_CMD_POS]) //处理命令
			{
			//	 case 0x30: BeginOutput1(); break;
				
						case 0x01: GetMachType();                     break;     //读取机器型号
						case 0x02: GetVersion();                      break;     //读取版本信息
						case 0x03: SetMachno();                       break;     //设置机号
						case 0x04: GetMachno();                       break;     //读取机号		
						case 0x05: SetTime();                         break;     //设置时间
						case 0x06: GetTime();                         break;     //读取时间
						case 0x07: SetSectionAndPassword();           break;     //设置读写扇区与密码	
						case 0x08: GetSectionAndPassword();           break;     //读取读写扇区与密码		
						case 0x09: Set_Clock_delay();		              break;     //设置打卡延时			
						case 0x0A: Set_Output_delay();		            break;     //设置开门延时				
						case 0x0B: Set_list_enabled();		            break;     //设置白名单是否开启		
						case 0x0C: Get_sysvar();		                  break;     //读取机器基本参数		
						case 0x0D: Set_Restore_factory();		          break;     //恢复出厂设置		
						case 0x0E: Set_ResNottore_factory();          break;     //恢复网络出厂设置						
						case 0x0F: Get_ChipInfo_Com();		            break;     //读取芯片信息			

				
				    case 0x10: GetBackWhiteNamesize();            break;     //读取黑白名单数量
						case 0x11: SetWhiteGroupName();               break;     //下发全部白名单
						case 0x12: GetWhiteGroupName();			          break;     //读取全部白名单
						case 0x13: ClearWhiteName();				          break;     //清除所有白名单
						case 0x14: AddOneWhiteGroupName();            break;     //追加单条白名单
						case 0x15: DelOneWhiteGroupName();            break;     //删除单条白名单
						case 0x16: SetBackGroupName();                break;     //下发全部黑名单		
						case 0x17: GetBackGroupName();                break;     //读取全部黑名单
						case 0x18: ClearBackName();                   break;     //清除全部黑名单
						case 0x19: AddOneBackGroupName();             break;     //追加一条黑名单		 
						case 0x1A: DelOneBackGroupName();             break;     //删除一条黑名单											
						case 0x1B: GetMaxRecordCount();               break;     //读取刷卡记录数量
						case 0x1C: GetRecord_Address();  		          break;     //读取新增刷卡记录
						case 0x1D: Get_MaxRecord();  				          break;     //读取全部刷卡记录
						case 0x1E: ClearRecord();                     break;     //清除所有刷卡记录

	//					case 0x21: Set_TCP_Mode();            break;     //设置TCP模式(DHCP or Static)		
						case 0x22: Set_This_MAC();                    break;     //设置本机Mac地址
						case 0x23: Get_This_MAC();                    break;     //读取本机Mac地址						
						case 0x24: Set_TCP_Local_IP();                break;     //设置本地IP地址		
						case 0x25: Get_TCP_Local_IP();                break;     //读取本地IP地址	
						case 0x26: Set_TCP_Remote_IP();               break;     //设置远端IP地址
						case 0x27: Get_TCP_Remote_IP();               break;     //读取远端IP地址		
						case 0x28: Set_TCP4G_Remote_IP();             break;     //设置4G远端IP地址       no
						case 0x29: Get_TCP4G_Remote_IP();             break;     //读取4G远端IP地址       no
						case 0x2E: Set_Open_Door();                   break;     //执行开门				
						case 0x2F: Set_NO_Open_Door();                break;     //执行不开门					


						case 0x30: Realtime_Upload_Record();          break;     //实时上传刷卡记录			      no
						case 0x31: Realtime_Upload_Return();          break;     //实时上传刷卡返回 						no	 			
						case 0x32: GetUpdataRecordCount();            break;     //获取实时上传的刷卡数量			  no
						case 0x33: GetUpdata_MaxRecord();             break;     //读取全部实时上传全部刷卡记录	no											
						case 0x34: ClearUpdataRecord();               break;     //清除实时上传数据 				    no
						case 0x35: UpdataRecordTEST();                break;     //测试						
						case 0x36: Get_Heartbeat_Send();              break;     //心跳发送                   no
						case 0x37: Heartbeat_Return();                break;     //心跳返回                   no
//						case 0x39: Network_Synchronization_Time();    break;     //网络同步时间


						case 0x40: Set_Interval_delay();              break;     //设置刷卡间隔时间
						case 0x41: Set_ReadorWriteICData_Mode();      break;     //设置是否读写卡扇区数据
						case 0x42: Set_Check_Sector_Password_Mode();  break;     //设置是否检测密码
						case 0x43: Get_Extvar();                      break;     //获取扩展参数
						case 0x44: Set_Permission_Period();           break;     //设置门禁时段
						case 0x45: Get_Permission_Period();           break;     //读取门禁时段
						case 0x46: Set_Passage_Time_Enable();         break;     //通道时段使能
						case 0x47: Set_Passage_Time();                break;     //设置通道时段
						case 0x48: Get_Passage_Time();                break;     //读取通道时段
						case 0x49: Set_Holiday_Vacations();           break;     //设置节假日日期
						case 0x4A: Get_Holiday_Vacations();           break;     //读取节假日日期



						case 0x61: Set_Sub_Maximum_Limit();  				  break;     //设定当次限额
						case 0x62: Get_Sub_Maximum_Limit();  				  break;     //读取当次限额						
						case 0x63: Set_Machine_Category();  				  break;     //设定扣费模式 01变值 02定值 03计次 04菜单
						case 0x64: Get_Machine_Category();  				  break;     //读取扣费模式 01变值 02定值 03计次 04菜单						
						case 0x65: Set_Fixed_consumption_Timer();  		break;     //设定就餐时段						
						case 0x66: Set_Fixed_consumption_Timer();  	  break;     //设定扩展就餐时段   未做
						case 0x67: Get_Fixed_consumption_Timer();  		break;     //读取就餐时段		
						case 0x68: Set_Fixed_consumption_Money();  		break;     //设定定值扣费金额						
						case 0x69: Set_Sub_Maximum_Limit();  				  break;     //设定扩展定值扣费金额  未做
						case 0x6A: Get_Fixed_consumption_Money();  	  break;     //读取定值扣费金额		
						case 0x6B: Set_Carte_Consum_Money();  				break;     //设定菜单扣费金额
						case 0x6C: Get_Carte_Consum_Money();  				break;     //读取菜单扣费金额
//						case 0x6D: Get_Carte_Consum_Money();  				break;     //余额查询上报
						case 0x6E: Query_Balance_Return();  				  break;     //余额查询返回
//						case 0x6F: Query_Balance_Return();  				  break;     //退款						

 //             0x71    0x72  用于预充值

						case 0x73: Get_Total_Consumption();  				  break;     //读取消费总额
						case 0x74: Set_Clean_Total_Consumption();  		break;     //清除消费总额

						case 0xA0: Set_IAP_Start_Update();  				  break;     //开始固件更新
						case 0xA1: Set_Handle_IAP_Data();  				    break;     //发送固件数据包
						case 0xA2: Set_Check_IAP_END_Updata();  			break;     //固件升级结束
						case 0xA3: Get_IAP_Read_Data();  				      break;     //读取固件内容						
						case 0xA4: Set_IAP_Updata_OK();  				      break;     //固件升级确认完成						
						
						case 0xF5: Get_GPS_Information();  				    break;     //读取GPS信息
						case 0xF6: AutoUP_GPS_Information();  				break;     //自动上报GPS信息

						case 0xFA: Get_IMEI_IMSI_Machno();            break;     //查询IMSI		


						case 0x51: TEST_GQ();  				                break;     //测试
			
						default:  goto com_end;
						
			}
			
			if((cmdbuf[0]!= 0x31)&&(cmdbuf[0]!= 0x37)&&(cmdbuf[0]!= 0xA1))
				{
						
				  Arrange_Record(0 ,1);
					
					AddRecord(0 ,0);                           //保存指令记录
					
					SaveOperation_CommCount();
				}
		  	
			com_end:
	       memset(communication_data,0,sizeof(communication_data));
			
		}	


		
		
		
		
		
		
		
		
		
		
		



