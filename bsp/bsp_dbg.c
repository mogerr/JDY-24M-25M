#include "bsp_dbg.h"
#include "bsp_ble.h"

uint8_t  dbg_datarev_buff[dbg_MAX_datarev_buffsize] = { 0U };
char DMA_dbg_buff2[dbg_MAX_datarev_buffsize] = { 0U };

/**
 * @brief dbg串口 阻塞性 发送字符串 单次发送
 * @param str 要发送的数据字符串
 */
void dbg_send(const uint8_t* str)
{
    uint16_t k = 0;
    do { k++; }
    while (*(str + k) != '\0');
    HAL_UART_Transmit(&DBG_UART, (const uint8_t*) (str), k, 100);
}


/**
 * @brief dbg串口 阻塞性 发送字符串 单次发送
 * @param str 要发送的数据字符串
 * @param Size 数据长度已知s
 */
void dbg_send2(const uint8_t* str, uint16_t Size)
{
    HAL_UART_Transmit(&DBG_UART, (const uint8_t*) (str), Size, 100);
}


/**
 * @brief dbg串口 non-blocking DMA send  单次发送
 * @param str 要发送的数据字符串
 */
void dbg_send_DMA(const uint8_t* str)
{
    uint16_t k = 0;
    do { k++; }
    while (*(str + k) != '\0');
    // DMA模式为NOMAL单次模式
    HAL_UART_Transmit_DMA(&DBG_UART, (const uint8_t*) str, k);
}



/**
 * @brief dbg串口 non-blocking DMA send  单次发送
 * @param str 要发送的数据字符串
 * @param Size 数据长度已知s
 */
void dbg_send2_DMA(const uint8_t* str, uint16_t Size)
{
    strcpy(DMA_dbg_buff2, (const char*) str);
    uint16_t tt = Size;
    // DMA模式为NOMAL单次模式
    HAL_UART_Transmit_DMA(&DBG_UART, (const uint8_t*) DMA_dbg_buff2, tt);
}

/**
 * @brief dbg串口 DMA方式接收不定长数据（开始接收）(单次接收)
 * @param
 */
void  dbg_recv_start_DMA(void)
{
    // 清零
    uint16_t k = 0;
    do { dbg_datarev_buff[k++] = '\0'; }
    while (*(dbg_datarev_buff + k) != '\0');
    // 空闲中断接收不定长数据
    // DMA模式为NOMAL单次模式
    HAL_UARTEx_ReceiveToIdle_DMA(&DBG_UART, dbg_datarev_buff, dbg_MAX_datarev_buffsize);
}







//重定向c库函数printf到串口USARTx，重定向后可使用printf函数
int fputc(int ch, FILE* f)
{
    /* 发送一个字节数据到串口USARTx */
    // HAL_UART_Transmit(&DBG_UART, (uint8_t *)&ch, 1, 1000);
    HAL_UART_Transmit(&DBG_UART, (uint8_t*) &ch, 1U, 200);
    return (ch);
}
//重定向c库函数scanf到串口USARTx，重写向后可使用scanf、getchar等函数
// int fgetc(FILE *f)
// {
//     int ch;
//     /* 等待串口输入数据 */
//     while (__HAL_UART_GET_FLAG(&DBG_UART, UART_FLAG_RXNE) == RESET);
//     // HAL_UART_Receive(&DBG_UART, (uint8_t *)&ch, 1, 0xFFFF);
//     HAL_UART_Receive(&DBG_UART, (uint8_t *)&ch, 1,1000);
//     return (ch);
// }




/**
 * @brief dbg 初始化
 * @param
 */
void dbg_main(void)
{
    dbg_recv_start_DMA();
}


void dbg_loop(void)
{

}
