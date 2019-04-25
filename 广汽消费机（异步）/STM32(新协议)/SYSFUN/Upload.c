#include "Head.h"

#include "Upload.h"



u8  UP_UserBalance[4]={0X00,0X00,0X00,0X00};	  //卡上余额
 
u8  UP_Consum_Amount[4] ={0X00,0X00,0X00,0X00};	  //消费金额

u8   Server_Returndata[30]={0};                //服务器返回的数据

u8  UP_defNOnetBalance[4]={0X38,0X90,0X0D,0X00};	  //无网络时默认余额     //低位在前

u8  UP_defNOretBalance[4]={0X88,0X88,0X88,0X88};	  //无网络时默认余额     //低位在前

u8  UP_defNOcardBalance[4]={0X00,0X00,0X00,0X00};	  //无IC卡时默认余额

uint8_t updatareturnfalg =0 ;  //实时上传返回标志

u16  upOverTimeCnt=0;                        //超时退出计数

u8   Card_Markok=0;                          //有刷卡的标记

u8   Local_Check_Card_falg=0;                //本地检查卡权限标志

u8   Offline_Online_falg=0;                      //消费时在线-离线标志 

/*********************************************

处理实时上传返回

**********************************************/
void  ProcessReal_time_upload(void)
{
//	uint8_t  i =0;
#ifdef   WISDOM_PLATE     //是智盘
	
uint8_t	WP_Pay_Carderr_Code =0;         //返回给智盘的错误代码         
	
#endif		
	
	
  if(Card_Markok ==0)      //如果没有刷卡 直接返回
	{
		
	return ;

	}
	
	if(Pay_Carderr_Code!=0)
	{
	 goto  play_card;
	}

	
	Local_Check_Card_falg=0;          //本都校验
	
	if(ec20_device_info.netWork)   //有网络
	{	

		if((updatareturnfalg ==0)&&(upOverTimeCnt))     //未收到服务器返回或者超时
				{
					return ;
				}	
				
		if(updatareturnfalg)    //服务器有返回
			{
				
	//			RS485_Send_Data(Server_Returndata,	30);
	/*********************************  拷贝服务器返回的数据		****************************************/	

				DelUPloadRecord(Server_Returndata);     //删除实时未上传数据
				
		    MyStrCPY(idbuff, Server_Returndata +4, CARD_SIZE);       //拷贝卡号				
				
				MyStrCPY(UserBalance, Server_Returndata +12, CARD_SIZE);        //拷贝卡上余额

		    MyStrCPY(Consum_Amount, Server_Returndata +8, CARD_SIZE);       //拷贝消费金额
	
				MyStrCPY(recorddata + 26, Server_Returndata, 4);           //流水号    4字节				
				
				MyStrCPY(recorddata + 25, recorddataup + 25, 1);           //通讯命令    1字节				
				
				Pay_Carderr_Code  =  Server_Returndata[17];      //返回错误代码赋值
				
				Offline_Online_falg =1;
							
			}
		else                           //服务器无返回
			{			
				Offline_Online_falg =0;
				
				MyStrCPY(recorddata + 4, recorddataup +4, 4);                //卡号 4字节
				MyStrCPY(recorddata + 26, recorddataup + 26, 4);           //流水号    4字节
				MyStrCPY(recorddata + 24, recorddataup + 24 , 1);          //读头号   1字节				
				MyStrCPY(recorddata + 25, recorddataup + 25, 1);           //通讯命令    1字节			
				
				
				if(GetWorkStatus(IN_CONSUMPT_STATUS))        //消费状态允许通过
					{
						
						MyStrCPY(Consum_Amount, keymoneybuff, CARD_SIZE);        //拷贝按键的金额

				    MyStrCPY(UserBalance, UP_defNOretBalance, CARD_SIZE);       //拷贝默认的金额	ffff
											
						Local_Check_Card_falg=1;    //本地检查刷卡权限
					}
				else if	(Machine_category[0] != 0x01)//定值扣费 
				{
						MyStrCPY(Consum_Amount +2, Fixed_consum_Moneybuf +Consum_TimerBit[1]*2, 2);        //拷贝定值的金额

				    MyStrCPY(UserBalance, UP_defNOretBalance, CARD_SIZE);       //拷贝默认的金额	ffff
											
						Local_Check_Card_falg=1;    //本地检查刷卡权限
				
				}			

				else if	(Machine_category[0] != 0x02)//计次扣费 
				{
						MyStrCPY(Consum_Amount , UP_defNOcardBalance, CARD_SIZE);        //拷贝计次的金额

				    MyStrCPY(UserBalance, UP_defNOretBalance, CARD_SIZE);       //拷贝默认的金额	ffff
											
						Local_Check_Card_falg=1;    //本地检查刷卡权限				
				}				
				else    //非消费状态显示错误
				{
						Pay_Carderr_Code = SYS_NET_CONNECT_FAIL;   //无网络
					
				}	

				
			}					
	}		
	else        //无网络
	{
		Offline_Online_falg =0;
		
		MyStrCPY(recorddata + 4, recorddataup +4, 4);                //卡号 4字节
		MyStrCPY(recorddata + 26, recorddataup + 26, 4);           //流水号    4字节
		MyStrCPY(recorddata + 24, recorddataup + 24 , 1);          //读头号   1字节				
		MyStrCPY(recorddata + 25, recorddataup + 25, 1);           //通讯命令    1字节			
		
		
		if(GetWorkStatus(IN_CONSUMPT_STATUS))        //消费状态 允许通过
			{	
				
				MyStrCPY(UserBalance,   UP_defNOretBalance, CARD_SIZE);        //拷贝默认的金额	1234

				MyStrCPY(Consum_Amount, keymoneybuff, CARD_SIZE);              // 拷贝按键的金额
				  
				Local_Check_Card_falg=1;    //本地检查刷卡权限
			}
		else if	(Machine_category[0] != 0x01)//定值扣费 
		{
				MyStrCPY(Consum_Amount+2, Fixed_consum_Moneybuf +Consum_TimerBit[1]*2, 2);        //拷贝定值的金额

				MyStrCPY(UserBalance, UP_defNOretBalance, CARD_SIZE);       //拷贝默认的金额	ffff
									
				Local_Check_Card_falg=1;    //本地检查刷卡权限		
		}			
			
		else if	(Machine_category[0] != 0x02)//计次扣费 
		{
				MyStrCPY(Consum_Amount, UP_defNOcardBalance, CARD_SIZE);        //拷贝计次的金额

				MyStrCPY(UserBalance, UP_defNOretBalance, CARD_SIZE);       //拷贝默认的金额	ffff
									
				Local_Check_Card_falg=1;    //本地检查刷卡权限		
		}					
	
		else
			{
				Pay_Carderr_Code = SYS_NO_NETWORK;
			}	
			
	}
	
	if(Local_Check_Card_falg)
	{		
		
	  Check_Card_Limit (Pay_Head_Code);      //检查刷卡权限
		
		if(GetWorkStatus(IN_CONSUMPT_STATUS)||(Machine_category[0] != 0x01)||(Machine_category[0] != 0x02)||(Machine_category[0] != 0x03))        //按键状态 保存实时上传记录
			{
				
				Add_UpdataRecord(0);          //保存实时上传记录				
			}		
	}
	
	if(GetWorkStatus(IN_CONSUMPT_STATUS)||(Machine_category[0] != 0x01)||(Machine_category[0] != 0x02)||(Machine_category[0] != 0x03))   //只有在消费状态或者定值才保存数据
		{
			 AddRecord(Pay_Head_Code ,0);          //存储记录
							
			 delay_ms(1);
			
			 UndoPressKEY();//撤消按键
			
			 if(Pay_Carderr_Code == PAY_OK)    //   
				{
				 SaveActual_CardCount(); 				 
				}
				else                         // 
				{				
				 SaveErr_CardCount();			//保存错误数卡数量
				}			
		}	
			
		
#ifdef   WISDOM_PLATE     //是智盘

		/****** 智盘返回错误码转换  ********/
		/**
        0 表示成功
        1 表示失败
        3 表示超过消费限额（比如超过单餐，或者单日消费限额）
        4 表示交易未决（由于抽卡过快，pos机不知道交易是否成功），返回未决指令后，结算台会进入未决状态，等待pos机再次返回，定义见后文
        5 卡被禁用
        6 需要验证密码（将会重新发起带密码的支付）
        10 卡已损坏
        14 余额不足
        105 设备不可用（尝试重新连接）
        111 设备结算区满，需要同步（停止交易，发起结算）

**/
		
			switch(Pay_Carderr_Code)    
				{
					case 0x00: WP_Pay_Carderr_Code= 0;   break;  //					
					case 0x01: WP_Pay_Carderr_Code= 1;   break;  //
					case 0x02: WP_Pay_Carderr_Code= 5;   break;  // 
					case 0x03: WP_Pay_Carderr_Code= 5;   break;  // 					
					case 0x04: WP_Pay_Carderr_Code= 5;   break;  // 
					case 0x05: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x06: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x07: WP_Pay_Carderr_Code= 1;   break;  // 
					
					case 0x10: WP_Pay_Carderr_Code= 105;  break;  // 
					case 0x11: WP_Pay_Carderr_Code= 105;  break;  // 					
					
					case 0x20: WP_Pay_Carderr_Code= 14;  break;  // 
					case 0x21: WP_Pay_Carderr_Code= 3;   break;  //  
					case 0x22: WP_Pay_Carderr_Code= 3;   break;  // 
					case 0x23: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x24: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x25: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x26: WP_Pay_Carderr_Code= 3;   break;  // 
					case 0x27: WP_Pay_Carderr_Code= 3;   break;  //						
					case 0xF1: WP_Pay_Carderr_Code= 0;   break;  //
					case 0xF2: WP_Pay_Carderr_Code= 0;   break;  //
					
					default:   WP_Pay_Carderr_Code= 0;  
					
				}
					
//	MyStrCPY(WP_Consum_Amount,  Consum_Amount, 4);       //智盘消费金额
				
   MyStrCPY(WP_UserBalance,    UserBalance, 4);         //智盘卡上余额
					
	 Wisdom_plate_Return(0X02,WP_Pay_Carderr_Code);          //响应智盘		有网络情况返回真实值，断网情况返回余额值0。
		
#else		     //不是智盘

#endif			
		
						
play_card:			
				
				
     LCD_ShowPlay_Card(Pay_Carderr_Code);       //LCD显示
	
	   Card_Markok=0;

     Enable_Pay_Card=1;   //允许刷卡			
				
		 idcycleflag =1;      //重复数卡间隔计数开始 2019-02-21
		 
		 	Asynchronous_QR_falg =0 ;      //客餐标志
		 
}





void  newCheck_Card_Limit(void)
{

	#ifdef   WISDOM_PLATE     //是智盘
	
uint8_t	WP_Pay_Carderr_Code =0;         //返回给智盘的错误代码         
	
#endif	
	
	  Check_Card_Limit (Pay_Head_Code);      //检查刷卡权限

	if(GetWorkStatus(IN_CONSUMPT_STATUS)||(Machine_category[0] != 0x01)||(Machine_category[0] != 0x02)||(Machine_category[0] != 0x03))   //只有在消费状态或者定值才保存数据
		{
	//		 Arrange_Record(Pay_Head_Code ,0);         //整理刷卡数据
			
			 AddRecord(Pay_Head_Code ,0);          //存储记录
						
		
//			 delay_ms(1);
			
			 UndoPressKEY();//撤消按键
			
			 if(Pay_Carderr_Code == PAY_OK)    //   
				{
				 Add_UpdataRecord(0);          //保存实时上传记录		
			
	       UPloadRecord(3);        //实时上传数据	
										
				 SaveActual_CardCount(); 				 
				}
				else                         // 
				{				
				 SaveErr_CardCount();			//保存错误数卡数量
				}			
		}	
			
		
#ifdef   WISDOM_PLATE     //是智盘

		/****** 智盘返回错误码转换  ********/
		/**
        0 表示成功
        1 表示失败
        3 表示超过消费限额（比如超过单餐，或者单日消费限额）
        4 表示交易未决（由于抽卡过快，pos机不知道交易是否成功），返回未决指令后，结算台会进入未决状态，等待pos机再次返回，定义见后文
        5 卡被禁用
        6 需要验证密码（将会重新发起带密码的支付）
        10 卡已损坏
        14 余额不足
        105 设备不可用（尝试重新连接）
        111 设备结算区满，需要同步（停止交易，发起结算）

**/
		
			switch(Pay_Carderr_Code)    
				{
					case 0x00: WP_Pay_Carderr_Code= 0;   break;  //					
					case 0x01: WP_Pay_Carderr_Code= 1;   break;  //
					case 0x02: WP_Pay_Carderr_Code= 5;   break;  // 
					case 0x03: WP_Pay_Carderr_Code= 5;   break;  // 					
					case 0x04: WP_Pay_Carderr_Code= 5;   break;  // 
					case 0x05: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x06: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x07: WP_Pay_Carderr_Code= 1;   break;  // 
					
					case 0x10: WP_Pay_Carderr_Code= 105;  break;  // 
					case 0x11: WP_Pay_Carderr_Code= 105;  break;  // 					
					
					case 0x20: WP_Pay_Carderr_Code= 14;  break;  // 
					case 0x21: WP_Pay_Carderr_Code= 3;   break;  //  
					case 0x22: WP_Pay_Carderr_Code= 3;   break;  // 
					case 0x23: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x24: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x25: WP_Pay_Carderr_Code= 1;   break;  // 
					case 0x26: WP_Pay_Carderr_Code= 3;   break;  // 
					case 0x27: WP_Pay_Carderr_Code= 3;   break;  //						
					case 0xF1: WP_Pay_Carderr_Code= 0;   break;  //
					case 0xF2: WP_Pay_Carderr_Code= 0;   break;  //
					
					default:   WP_Pay_Carderr_Code= 0;  
					
				}
					
//	MyStrCPY(WP_Consum_Amount,  Consum_Amount, 4);       //智盘消费金额
				
   MyStrCPY(WP_UserBalance,    UserBalance, 4);         //智盘卡上余额
					
	 Wisdom_plate_Return(0X02,WP_Pay_Carderr_Code);          //响应智盘		有网络情况返回真实值，断网情况返回余额值0。
		
#else		     //不是智盘

#endif			
		
										
     LCD_ShowPlay_Card(Pay_Carderr_Code);       //LCD显示
	
	   Card_Markok=0;

     Enable_Pay_Card=1;   //允许刷卡			
				
		 idcycleflag =1;      //重复数卡间隔计数开始 2019-02-21

}














