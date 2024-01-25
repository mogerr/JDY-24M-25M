#ifndef BSP_BLE_H
#define BSP_BLE_H

#include "usart.h"
#include "stdio.h"
#include "string.h"

#define  BLE_UART huart1
#define  ble_MAX_datarev_buffsize   (uint16_t)128U

extern uint8_t  ble_datarev_buff[ble_MAX_datarev_buffsize];

void ble_main(void);
void ble_loop(void);


void ble_send(const uint8_t* str);
void ble_send2(const uint8_t* str, uint16_t Size);
void ble_send_DMA(const uint8_t* str);
void ble_send2_DMA(const uint8_t* str, uint16_t Size);
void ble_recv_start_DMA(void);

#endif
