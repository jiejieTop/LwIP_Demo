/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   eth
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 STM32 F429 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */

#include "main.h"
#include "string.h"
#include "./eth/bsp_eth.h" 

#include "./usart/bsp_debug_usart.h"
#include "./led/bsp_led.h" 
#include "./eth/bsp_eth.h" 
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Global Ethernet handle */
ETH_TxPacketConfig TxConfig; 

ETH_HandleTypeDef heth;

UART_HandleTypeDef huart1;


#if defined ( __ICCARM__ ) /*!< IAR Compiler */

#pragma location=0x30040000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30040060
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
#pragma location=0x30040200
uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffers */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
__attribute__((at(0x30040200))) uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffer */

#elif defined ( __GNUC__ ) /* GNU Compiler */ 

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE] __attribute__((section(".RxArraySection"))); /* Ethernet Receive Buffers */

#endif


void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* 使能端口时钟 */
  ETH_MDIO_GPIO_CLK_ENABLE();
  ETH_MDC_GPIO_CLK_ENABLE();
  ETH_RMII_REF_CLK_GPIO_CLK_ENABLE();
  ETH_RMII_CRS_DV_GPIO_CLK_ENABLE();
  ETH_RMII_RXD0_GPIO_CLK_ENABLE();
  ETH_RMII_RXD1_GPIO_CLK_ENABLE();
  ETH_RMII_TX_EN_GPIO_CLK_ENABLE();
  ETH_RMII_TXD0_GPIO_CLK_ENABLE();    
  ETH_RMII_TXD1_GPIO_CLK_ENABLE();

  /* 配置ETH_MDIO引脚 */
  GPIO_InitStructure.Pin = ETH_MDIO_PIN;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Alternate = ETH_MDIO_AF;
  HAL_GPIO_Init(ETH_MDIO_PORT, &GPIO_InitStructure);

  /* 配置ETH_MDC引脚 */
  GPIO_InitStructure.Pin = ETH_MDC_PIN;
  GPIO_InitStructure.Alternate = ETH_MDC_AF;
  HAL_GPIO_Init(ETH_MDC_PORT, &GPIO_InitStructure);

  /* 配置ETH_RMII_REF_CLK引脚 */
  GPIO_InitStructure.Pin = ETH_RMII_REF_CLK_PIN;
  GPIO_InitStructure.Alternate = ETH_RMII_REF_CLK_AF;
  HAL_GPIO_Init(ETH_RMII_REF_CLK_PORT, &GPIO_InitStructure);

  /* 配置ETH_RMII_CRS_DV引脚 */
  GPIO_InitStructure.Pin = ETH_RMII_CRS_DV_PIN;
  GPIO_InitStructure.Alternate = ETH_RMII_CRS_DV_AF;
  HAL_GPIO_Init(ETH_RMII_CRS_DV_PORT, &GPIO_InitStructure);

  /* 配置ETH_RMII_RXD0引脚 */
  GPIO_InitStructure.Pin = ETH_RMII_RXD0_PIN;
  GPIO_InitStructure.Alternate = ETH_RMII_RXD0_AF;
  HAL_GPIO_Init(ETH_RMII_RXD0_PORT, &GPIO_InitStructure);

  /* 配置ETH_RMII_RXD1引脚 */
  GPIO_InitStructure.Pin = ETH_RMII_RXD1_PIN;
  GPIO_InitStructure.Alternate = ETH_RMII_RXD1_AF;
  HAL_GPIO_Init(ETH_RMII_RXD1_PORT, &GPIO_InitStructure);

  /* 配置ETH_RMII_TX_EN引脚 */
  GPIO_InitStructure.Pin = ETH_RMII_TX_EN_PIN;
  GPIO_InitStructure.Alternate = ETH_RMII_TX_EN_AF;
  HAL_GPIO_Init(ETH_RMII_TX_EN_PORT, &GPIO_InitStructure);

  /* 配置ETH_RMII_TXD0引脚 */
  GPIO_InitStructure.Pin = ETH_RMII_TXD0_PIN;
  GPIO_InitStructure.Alternate = ETH_RMII_TXD0_AF;
  HAL_GPIO_Init(ETH_RMII_TXD0_PORT, &GPIO_InitStructure);

  /* 配置ETH_RMII_TXD1引脚 */
  GPIO_InitStructure.Pin = ETH_RMII_TXD1_PIN;
  GPIO_InitStructure.Alternate = ETH_RMII_TXD1_AF;
  HAL_GPIO_Init(ETH_RMII_TXD1_PORT, &GPIO_InitStructure);      
  
    /* 使能以太网时钟  */
  __HAL_RCC_ETH1MAC_CLK_ENABLE();
  __HAL_RCC_ETH1TX_CLK_ENABLE();
  __HAL_RCC_ETH1RX_CLK_ENABLE();  
  
}

static void Eth_Reset(ETH_HandleTypeDef *heth)
{ 
  HAL_ETH_WritePHYRegister(heth, LAN8720A_PHY_ADDRESS, PHY_BCR, PHY_RESET);
}

/**
* @brief ETH MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param heth: ETH handle pointer
* @retval None
*/
void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{
  if(heth->Instance==ETH)
  {
  /* USER CODE BEGIN ETH_MspDeInit 0 */

  /* USER CODE END ETH_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ETH1MAC_CLK_DISABLE();
    __HAL_RCC_ETH1TX_CLK_DISABLE();
    __HAL_RCC_ETH1RX_CLK_DISABLE();
  
    /**ETH GPIO Configuration    
    PG12     ------> ETH_TXD1
    PG13     ------> ETH_TXD0
    PC1     ------> ETH_MDC
    PA2     ------> ETH_MDIO
    PA1     ------> ETH_REF_CLK
    PB11     ------> ETH_TX_EN
    PA7     ------> ETH_CRS_DV
    PC4     ------> ETH_RXD0
    PC5     ------> ETH_RXD1 
    */
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_12|GPIO_PIN_13);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);

  /* USER CODE BEGIN ETH_MspDeInit 1 */

  /* USER CODE END ETH_MspDeInit 1 */
  }

}


HAL_StatusTypeDef Bsp_Eth_Init(void)
{
  uint32_t idx = 0;
  //mac地址
  uint8_t macaddress[6]= {MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5};   
  
  heth.Instance = ETH;  
  heth.Init.MACAddr = macaddress;
  //RMII模式
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  //接受描述符
  heth.Init.RxDesc = DMARxDscrTab;
  //发送描述符
  heth.Init.TxDesc = DMATxDscrTab;
  //数据长度
  heth.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;
  
  /* 配置以太网外设 (GPIOs, clocks, MAC, DMA)*/
  HAL_ETH_Init(&heth);
  
  HAL_ETH_DeInit(&heth);
  
  Eth_Reset(&heth);
  
/* 设置netif MAC 硬件地址长度 */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  
  /* 设置netif MAC 硬件地址 */
  netif->hwaddr[0] =  MAC_ADDR0;
  netif->hwaddr[1] =  MAC_ADDR1;
  netif->hwaddr[2] =  MAC_ADDR2;
  netif->hwaddr[3] =  MAC_ADDR3;
  netif->hwaddr[4] =  MAC_ADDR4;
  netif->hwaddr[5] =  MAC_ADDR5;
  
  /* 设置netif最大传输单位 */
  netif->mtu = ETH_MAX_PAYLOAD;
  
  /* 接收广播地址和ARP流量  */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  
  for(idx = 0; idx < ETH_RX_DESC_CNT; idx ++)
  {
    HAL_ETH_DescAssignMemory(&EthHandle, idx, Rx_Buff[idx], NULL);
    
    /* Set Custom pbuf free function */
    rx_pbuf[idx].custom_free_function = pbuf_free_custom;
  }
  
  /* 设置发送配置结构体 */
  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  //发送校验
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  //CRC校验位
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  //初始化LAN8720A
  if(LAN8720_Init(&EthHandle) == HAL_OK) 
  {    
      ethernet_link_check_state(netif);
  }
  /* Init ETH */
  MACAddr[0] = 0x02;
  MACAddr[1] = 0x00;
  MACAddr[2] = 0x00;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Instance = ETH;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;
  
	/* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
  ret = HAL_ETH_Init(&heth);
	if (ret == HAL_OK)
		PRINT_DEBUG("eth hardware init sucess...\n");
  else
    PRINT_DEBUG("eth hardware init faild...\n");
    
  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  
  return ret;
}


void ETH_IRQHandler(void)
{
  uint32_t ulReturn;
  /* 进入临界段，临界段可以嵌套 */
  ulReturn = taskENTER_CRITICAL_FROM_ISR();
  
  HAL_ETH_IRQHandler(&heth);
  
  /* 退出临界段 */
  taskEXIT_CRITICAL_FROM_ISR( ulReturn );
}

/**
  * @brief  Ethernet Rx Transfer completed callback
  * @param  heth: ETH handle
  * @retval None
  */
extern xSemaphoreHandle s_xSemaphore;
void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
  LED2_TOGGLE;
//  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
//  xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );
//  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
  ;
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    PRINT_ERR("eth err\n");
}
