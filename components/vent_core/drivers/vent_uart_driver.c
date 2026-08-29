#include "vent_uart_driver.h"

#include <freertos/FreeRTOS.h>

#define UART_EVENT_QUEUE_DEPTH 16

esp_err_t vent_uart_driver_init(
  uart_port_t port,
  gpio_num_t rx_pin,
  gpio_num_t tx_pin,
  uint32_t baud_rate,
  size_t rx_buffer_size,
  uint8_t rx_idle_byte_times,
  QueueHandle_t *out_event_queue) {
  esp_err_t err = uart_driver_install(port, (int)rx_buffer_size, 0, UART_EVENT_QUEUE_DEPTH, out_event_queue, 0);
  if (err != ESP_OK) return err;

  const uart_config_t cfg = {
    .baud_rate = (int)baud_rate,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };
  err = uart_param_config(port, &cfg);
  if (err != ESP_OK) return err;

  err = uart_set_pin(port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  if (err != ESP_OK) return err;

  // The RX idle timeout raises a UART_DATA event on the bus gap between frames,
  // so the reader task gets one event per frame instead of polling for bytes
  return uart_set_rx_timeout(port, rx_idle_byte_times);
}

int vent_uart_driver_read_bytes(uart_port_t port, uint8_t *buf, size_t max_len, uint32_t timeout_ms) {
  return uart_read_bytes(port, buf, max_len, pdMS_TO_TICKS(timeout_ms));
}
