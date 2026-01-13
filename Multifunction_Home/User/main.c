#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "tftlcd.h"



#include "dht11.h"
#include "lsens.h"
#include "pwm.h"
#include "hwjs.h"
#include "step.h"
#include "beep.h"
#include "image_flame.h"
#include "flame.h"
#include "rgb.h"
#include "f1.h"
#include "landscope.h"
#include "landscope2.h"


u8 i=0;
u16 color=0;
u8 temp;
u8 humi;
u8 temp_buf[3],humi_buf[3];
u8 lsens_value=0;
u8 Lsens[3];//光感参数
u16 j=0;
u8 fx=0;//呼吸灯参数


void Lightt(void);
void Temp_Humi(void);
void BreatheLED(void);
u8 Hwjs(void);
void Flame(void);
void RGB(void);


/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int main()
{
	
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
	LED_Init();
	USART1_Init(115200);
	TFTLCD_Init();			//LCD初始化
	
	
	DHT11_Init();
	Lsens_Init();
	Hwjs_Init();//红外初始化
	Step_Motor_GPIO_Init();
	BEEP_Init();//蜂鸣器初始化
	Flame_Init();
	RGB_Init();
	
	
		FRONT_COLOR=BLACK;
		color=LCD_ReadPoint(20,160);
		//LCD_Fill(100,150,150,180,color);
		printf("color=%x\r\n",color);
		
		LCD_ShowPicture(20,170,275,279,(u8 *)gImage_landscope);
		
		while(1)
		{
			i++;
			if(i%20==0)
			{
				LED1=!LED1;
			}

			Lightt();
			Temp_Humi();
			
			BreatheLED();
			Hwjs();
			Flame();
			delay_ms(20);
		}
}



void Lightt(){
		lsens_value=Lsens_Get_Val();
		//printf("光照强度 = %d\n",lsens_value);
		
    Lsens[0] = lsens_value/10+0x30;
    Lsens[1] = lsens_value%10+0x30;

		LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,24,"Light Sense: ");
    LCD_ShowString(160,10,tftlcd_data.width,tftlcd_data.height,24,Lsens);
		//printf("光照强度 = %c,%c\n",Lsens[0],Lsens[1]);
	
		if(lsens_value < 10){
			motor_circle(64, 0, 2);
		}
}



void Temp_Humi(){
	
		DHT11_Read_Data(&temp,&humi);
		temp_buf[0]=temp/10+0x30;
		temp_buf[1]=temp%10+0x30;
		temp_buf[2]='\0';
		LCD_ShowString(10,40,tftlcd_data.width,tftlcd_data.height,24,"Temp:   C");
		LCD_ShowString(80,40,tftlcd_data.width,tftlcd_data.height,24,temp_buf);

		//printf("temp = %c, %c\n",temp_buf[0],temp_buf[1]);

		humi_buf[0]=humi/10+0x30;
		humi_buf[1]=humi%10+0x30;
		humi_buf[2]='\0';
		LCD_ShowString(10,70,tftlcd_data.width,tftlcd_data.height,24,"Humi:   %RH");
		LCD_ShowString(80,70,tftlcd_data.width,tftlcd_data.height,24,humi_buf);
		//printf("humi = %c, %c\n",humi_buf[0],humi_buf[1]);
		//printf("温度=%d°C  湿度=%d%%RH\r\n",temp,humi);
}



void BreatheLED()//呼吸灯模块
{
    SysTick_Init(72);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
    LED_Init();
    TIM3_CH2_PWM_Init(500,72-1); //频率是2Kh
    if(fx==0)
    {
        j+=25;
        if(j==300)
        {
            fx=1;
        }
    }
    else
    {
        j-=25;
        if(j==0)
        {
            fx=0;
        }
    }
    TIM_SetCompare2(TIM3,j);  //i值最大可以取499，因为ARR最大值是499.
    delay_ms(10);
}


u8 Hwjs(){
//红外模块
        if(hw_jsbz==1)
        {
            hw_jsbz=0;
            if(HW_DMZY()==1)
            {
                printf("电源 \n");
								return 1;
            }
            else if(HW_DMZY()==2)
            {
                printf("按键 0\n");
								motor_circle(64, 1, 2);
            }
            else if(HW_DMZY()==3)
            {
                printf("按键 1\n");
								motor_circle(64, 0, 2);
            }
            else if(HW_DMZY()==4)
            {
                printf("mode \n");
            }
						else if(HW_DMZY()==5)
            {
                printf("按键 4 \n");
								LCD_Clear(WHITE);
								LCD_ShowPicture(40,180,240,160,(u8 *)gImage_f1);
            }else if(HW_DMZY()==6)
            {
                printf("按键 5 \n");
								LCD_Clear(WHITE);
								LCD_ShowPicture(20,170,275,279,(u8 *)gImage_landscope);
            }else if(HW_DMZY()==7)
            {
                printf("按键 6 \n");
								LCD_Clear(WHITE);
								LCD_ShowPicture(50,170,213,281,(u8 *)gImage_landscope2);
            }else if(HW_DMZY()==8)
            {
                printf("按键 3 \n");
								LCD_Clear(WHITE);
            }
            hw_jsm=0;
        }
				return 0;
}


void Flame(){
		if(GPIO_ReadInputDataBit(PA,TAP_GPIO)==1)
		{
			BEEP = 1;
			LCD_Clear(WHITE);
			LCD_ShowPicture(90,150,150,150,(u8 *)gImage_image_flame);
			RGB();
			GPIO_SetBits(LEDS_GPIO_PORT,LEDS_GPIO_PIN);
			delay_ms(10);
		}
		else
		{
			GPIO_ResetBits(LEDS_GPIO_PORT,LEDS_GPIO_PIN);
			delay_ms(500);
		}
}


void RGB(){
		while(1){
			LED_SetRGB(255,0,0);//红色
			delay_ms(100);
		
			LED_SetRGB(0,255,0);//绿色
			delay_ms(100);
		
			LED_SetRGB(0,0,255);//蓝色
			delay_ms(100);
		
			LED_SetRGB(255,255,0);//黄色
			delay_ms(100);
		
			LED_SetRGB(0,255,255);//青色
			delay_ms(100);
		
			LED_SetRGB(255,0,255);//紫红色
			delay_ms(100);
		
			/*LED_SetRGB(192,192,192);//银
			delay_ms(100);
		
			LED_SetRGB(128,128,128);//灰色
			delay_ms(100);
		
			LED_SetRGB(128,0,0);//	栗色
			delay_ms(100);
		
			LED_SetRGB(128,128,0);//橄榄色
			delay_ms(100);
			
			LED_SetRGB(0,128,0);//绿色
			delay_ms(100);
			
			LED_SetRGB(128,0,128);//紫色
			delay_ms(100);
			
			LED_SetRGB(0,128,128);//蓝绿色
			delay_ms(100);
			
			LED_SetRGB(0,0,128);//海军色
			delay_ms(100);*/
			
			LED_SetRGB(0,0,0);
			delay_ms(100);
			
			//LED_SetRGB(255,255,255);
			//delay_ms(100);
			
			if(Hwjs()){
					BEEP = 0;
					LCD_Clear(WHITE);
					LCD_ShowPicture(20,170,275,279,(u8 *)gImage_landscope);
					LED_SetRGB(0,0,0);
					break;
			}
		}
}



