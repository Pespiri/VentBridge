#pragma once

#include <driver/uart.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <hal/gpio_types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Install and configure a UART port for 8N1 operation
 *
 * @param[in]   port                 UART port to configure
 * @param[in]   rx_pin               RX GPIO
 * @param[in]   tx_pin               TX GPIO
 * @param[in]   baud_rate            baud rate
 * @param[in]   rx_buffer_size       size of the driver's internal RX ring buffer
 * @param[in]   rx_idle_byte_times   idle time, in byte transmission times, that ends a received frame
 * @param[out]  out_event_queue      receives the driver's event queue handle
 */
esp_err_t vent_uart_driver_init(uart_port_t port, gpio_num_t rx_pin, gpio_num_t tx_pin, uint32_t baud_rate, size_t rx_buffer_size, uint8_t rx_idle_byte_times, QueueHandle_t *out_event_queue);

/**
 * @brief Read up to `max_len` bytes from `port`, waiting at most `timeout_ms`
 *
 * @return number of bytes read (may be 0 on timeout), or -1 on error
 */
int vent_uart_driver_read_bytes(uart_port_t port, uint8_t *buf, size_t max_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
