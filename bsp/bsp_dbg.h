#ifndef BSP_DBG_H
#define BSP_DBG_H

#include "usart.h"
#include "stdio.h"
#include "dma.h"
#include "string.h"

#define DBG_UART huart2
#define dbg_MAX_datarev_buffsize  (uint16_t)128U

extern uint8_t  dbg_datarev_buff[dbg_MAX_datarev_buffsize];



void dbg_main(void);
void dbg_loop(void);


void dbg_send(const uint8_t* str);
void dbg_send2(const uint8_t* str, uint16_t Size);
void dbg_send_DMA(const uint8_t* str);
void dbg_send2_DMA(const uint8_t* str, uint16_t Size);
void  dbg_recv_start_DMA(void);


#endif
