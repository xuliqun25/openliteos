/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   UDP Client 例程
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F429 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "./Bsp/led/bsp_led.h" 
#include "./Bsp/usart/bsp_debug_usart.h"
#include "./Bsp/systick/bsp_SysTick.h"
#include "./Bsp/key/bsp_key.h"
#include "lwip/tcp.h"
#include "netconf.h"
#include "LAN8742A/LAN8742A.h"
#include "udp_echoclient.h"
#include "main.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
KEY Key1,Key2;
struct netif g_netif;

extern __IO uint8_t EthLinkStatus;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
/* Private function prototypes -----------------------------------------------*/
extern err_t ethernetif_init(struct netif *netif);

static void TIM3_Config(uint16_t period,uint16_t prescaler);
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
UINT32 osAppInit(void)
{  
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
	
	struct sockaddr_in client_addr;  
	int sock_fd; 			   /* client socked */	
	int err;  
	
	char udp_msg[] = "this is a UDP test package";  

	/* 初始化LED */
	LED_GPIO_Config();
	
	Key1_GPIO_Config();
	Key2_GPIO_Config();
	KeyCreate(&Key1,GetPinStateOfKey1);
	KeyCreate(&Key2,GetPinStateOfKey2);
	
	/* 初始化调试串口，一般为串口1 */
	Debug_USART_Config();
	
	/* 初始化系统滴答定时器 */	
	SysTick_Init();
	
	TIM3_Config(999,899);//10ms定时器
	printf("LAN8720A Ethernet Demo\n");

	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
  ETH_BSP_Config();	
  printf("LAN8720A BSP INIT AND COMFIGURE SUCCESS\n");
	
  /* Initilaize the LwIP stack */
  //LwIP_Init();	
    tcpip_init(NULL, NULL);
    IP_ADDR4(&ipaddr,192,168,0,140);
    IP_ADDR4(&netmask,255,255,255,0);
    IP_ADDR4(&gw,192,168,0,1);
    netif_add(&g_netif, &ipaddr, &netmask, &gw, NULL, 
      &ethernetif_init, &tcpip_input);
    netif_set_default(&g_netif);
  
    if (netif_is_link_up(&g_netif))
    {
        netif_set_up(&g_netif);
    }
    else
    {
        netif_set_down(&g_netif);
    }
    

		 
	  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);  
    if (sock_fd == -1) {  
        printf("failed to create sock_fd!\n");	
	      return (UINT32)-1;
	  }  
	 	 
	  memset(&client_addr, 0, sizeof(client_addr));  
	  client_addr.sin_family = AF_INET;
	  client_addr.sin_addr.s_addr = inet_addr("192.168.0.100");  
	  client_addr.sin_port = htons(60000);	
		 
	
    err = sendto(sock_fd, (char *)udp_msg, sizeof(udp_msg), 0,  
	    (struct sockaddr *)&client_addr, sizeof(client_addr));  
		printf("err is %d\n",err);
	  return 0;
}

/**
  * @brief  通用定时器3中断初始化
  * @param  period : 自动重装值。
  * @param  prescaler : 时钟预分频数
  * @retval 无
  * @note   定时器溢出时间计算方法:Tout=((period+1)*(prescaler+1))/Ft us.
  *          Ft=定时器工作频率,为SystemCoreClock/2=90,单位:Mhz
  */
static void TIM3_Config(uint16_t period,uint16_t prescaler)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=prescaler;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=period;   //自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  定时器3中断服务函数
  * @param  无
  * @retval 无
  */
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
		LocalTime+=10;//10ms增量
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
