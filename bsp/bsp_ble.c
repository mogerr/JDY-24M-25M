#include "bsp_ble.h"
#include "bsp_dbg.h"
#include "jdy.h"

uint8_t  ble_datarev_buff[ble_MAX_datarev_buffsize] = { 0U };


char DMA_ble_buff2[ble_MAX_datarev_buffsize] = { 0U };

/**
 * @brief ble串口 阻塞性 发送字符串 单次发送
 * @param str 要发送的数据字符串
 */
void ble_send(const uint8_t* str)
{
    uint16_t k = 0;
    do { k++; }
    while (*(str + k) != '\0');
    HAL_UART_Transmit(&BLE_UART, (uint8_t*) (str), k, 100);
}


/**
 * @brief ble串口 阻塞性 发送字符串 单次发送
 * @param str 要发送的数据字符串
 * @param Size 数据长度已知s
 */
void ble_send2(const uint8_t* str, uint16_t Size)
{
    HAL_UART_Transmit(&BLE_UART, (uint8_t*) str, Size, 100);
}


/**
 * @brief ble串口 non-blocking DMA send  单次发送
 * @param str 要发送的数据字符串
 */
void ble_send_DMA(const uint8_t* str)
{
    uint16_t k = 0;
    do { k++; }
    while (*(str + k) != '\0');
    // DMA模式为NOMAL单次模式
    HAL_UART_Transmit_DMA(&BLE_UART, (const uint8_t*) str, k);
}

/**
 * @brief ble串口 non-blocking DMA send  单次发送
 * @param str 要发送的数据字符串
 * @param Size 数据长度已知
 */
void ble_send2_DMA(const uint8_t* str, uint16_t Size)
{
    strcpy(DMA_ble_buff2, (const char*) str);
    uint16_t tt = Size;
    // DMA模式为NOMAL单次模式
    HAL_UART_Transmit_DMA(&BLE_UART, (const uint8_t*) DMA_ble_buff2, tt);
}


/**
 * @brief ble串口 DMA方式接收不定长数据（开始接收）(单次接收)
 * @param
 */
void  ble_recv_start_DMA(void)
{
    // 清零
    uint16_t k = 0;
    do { ble_datarev_buff[k++] = '\0'; }
    while (*(ble_datarev_buff + k) != '\0');
    // 空闲中断接收不定长数据
    // DMA模式为NOMAL单次模式
    HAL_UARTEx_ReceiveToIdle_DMA(&BLE_UART, ble_datarev_buff, ble_MAX_datarev_buffsize);
}






/**
 * @brief ble 测试用 初始化
 * @param
 */
void ble_main(void)
{
    ble_recv_start_DMA();
}

void ble_loop(void)
{

}

// void ble_init(void)
// {
//     // JDY.BAUD = B9600;
//     // JDY.ENLOG = on;
//     // JDY.

// }
