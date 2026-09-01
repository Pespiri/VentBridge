#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

#define AIRFLOW_UP_PIN                GPIO_NUM_2
#define AIRFLOW_DOWN_PIN              GPIO_NUM_3
#define AIR_TEMP_UP_PIN               GPIO_NUM_4
#define AIR_TEMP_DOWN_PIN             GPIO_NUM_5
#define FILTER_PIN                    GPIO_NUM_6

#define UART_RX_PIN                   GPIO_NUM_8
#define UART_TX_PIN                   GPIO_NUM_9

#define PANEL_UART_PORT               UART_NUM_1
#define PANEL_UART_BAUD_RATE          4800
#define PANEL_UART_RX_BUF_SIZE        1024
#define PANEL_UART_RX_IDLE_BYTE_TIMES 2 // Idle gap that ends a frame, in byte times (~2.1ms each at 4800 8N1).
#define PANEL_UART_READ_TIMEOUT_MS    20

/* Board wiring for this firmware. Alternate front-ends (ESPHome, Zigbee, Matter)
 * supply their own config rather than including this header. */
#define VENT_PANEL_READER_CONFIG_DEFAULT                 \
  {                                                      \
    .uart_port = PANEL_UART_PORT,                        \
    .rx_pin = UART_RX_PIN,                               \
    .tx_pin = UART_TX_PIN,                               \
    .baud_rate = PANEL_UART_BAUD_RATE,                   \
    .rx_buffer_size = PANEL_UART_RX_BUF_SIZE,            \
    .rx_idle_byte_times = PANEL_UART_RX_IDLE_BYTE_TIMES, \
    .read_timeout_ms = PANEL_UART_READ_TIMEOUT_MS,       \
  }

#define VENT_BUTTON_PINS_DEFAULT        \
  {                                     \
    .airflow_up = AIRFLOW_UP_PIN,       \
    .airflow_down = AIRFLOW_DOWN_PIN,   \
    .air_temp_up = AIR_TEMP_UP_PIN,     \
    .air_temp_down = AIR_TEMP_DOWN_PIN, \
    .filter = FILTER_PIN,               \
  }

/** Task priorities */
typedef enum TASK_PRIORITIES_ENUM {
  TASK_PRIORITY_IDLE = 0, // freeRTOS idle task priority (tskIDLE_PRIORITY)

  TASK_PRIORITY_CONSOLE = 1,
  TASK_PRIORITY_UART_READER = 2,
  TASK_PRIORITY_BUTTON_CONTROL = 3,
} task_priority_t;
