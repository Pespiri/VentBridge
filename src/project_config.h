#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

#define FAN_SPEED_UP_PIN              GPIO_NUM_2
#define FAN_SPEED_DOWN_PIN            GPIO_NUM_3
#define TEMP_UP_PIN                   GPIO_NUM_4
#define TEMP_DOWN_PIN                 GPIO_NUM_5
#define FILTER_PIN                    GPIO_NUM_6

#define UART_RX_PIN                   GPIO_NUM_9
#define UART_TX_PIN                   GPIO_NUM_10

#define PANEL_UART_PORT               UART_NUM_1
#define PANEL_UART_BAUD_RATE          4800
#define PANEL_UART_RX_BUF_SIZE        1024
#define PANEL_UART_RX_IDLE_BYTE_TIMES 2 // Idle gap that ends a frame, in byte times (~2.1ms each at 4800 8N1).
#define PANEL_UART_READ_TIMEOUT_MS    20

/** Task priorities */
typedef enum TASK_PRIORITIES_ENUM {
  TASK_PRIORITY_IDLE = 0, // freeRTOS idle task priority (tskIDLE_PRIORITY)

  TASK_PRIORITY_CONSOLE = 1,
  TASK_PRIORITY_UART_READER = 2,
  TASK_PRIORITY_BUTTON_CONTROL = 3,
} task_priority_t;
