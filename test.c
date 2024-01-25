
/* MX Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "bsp_dbg.h"
#include "bsp_ble.h"
#include "jdy.h"

/* USER CODE END Includes */


int main(void)
{
  /* MX BEGIN 1 */
  HAL_Init();
  SystemClock_Config();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* MX END 1 */
  
  /* USER CODE BEGIN 2 */
  HAL_Delay(100);
  dbg_main();
  ble_main();
  HAL_Delay(100);
  jdy_init();
  /* USER CODE END 2 */

  while (1)
  {

    jdy_loop();
	
  }
  /* USER CODE END 3 */
}

/**
 * @brief ���� ��дRX�ж��¼��ص�����
 * @param huart UART handle
 * @param Size  Number of data available in application reception buffer (indicates a position in
 *               reception buffer until which, data are available) ��ʾ���ܵ����������ݵĳ�����Size
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size)
{
    // dbg����
    if (huart == &DBG_UART)
    {   // ��ʲô 

        // ����   ���͵�dbg����
        // dbg_send2_DMA(dbg_datarev_buff, Size);
        // ble_send((uint8_t*)dbg_datarev_buff );

        // JDY �жϴ��� ���ñ��λ �� JDY �ӹ�
        jdy_it_dbg_handle();

        // �ٴο���DMA�����������жϽ���
        dbg_recv_start_DMA();
    }
    // ble����
    else if (huart == &BLE_UART)
    {   // ��ʲô
        
        // ����   ���͵�dbg����
        dbg_send("\r\n----BT:\r\n");
        dbg_send2_DMA(ble_datarev_buff, Size);

        //JDY �жϴ��� �� JDY �ӹ�
        jdy_it_ble_handle();

        // �ٴο���DMA�����������жϽ���
        ble_recv_start_DMA();
    }
}


