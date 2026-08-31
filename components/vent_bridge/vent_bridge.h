#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include <atomic>

extern "C" {
#include "drivers/vent_button_control.h"
#include "panel/vent_panel_reader.h"
}

namespace esphome {
  namespace vent_bridge {
    class VentBridge : public Component {
    public:
      void set_uart_port(uint8_t port) { this->reader_config_.uart_port = (uart_port_t)port; }
      void set_rx_pin(uint8_t pin) { this->reader_config_.rx_pin = (gpio_num_t)pin; }
      void set_tx_pin(uint8_t pin) { this->reader_config_.tx_pin = (gpio_num_t)pin; }
      void set_baud_rate(uint32_t baud) { this->reader_config_.baud_rate = baud; }

      void set_fan_up_pin(uint8_t pin) { this->button_pins_.fan_up = (gpio_num_t)pin; }
      void set_fan_down_pin(uint8_t pin) { this->button_pins_.fan_down = (gpio_num_t)pin; }
      void set_temp_up_pin(uint8_t pin) { this->button_pins_.temp_up = (gpio_num_t)pin; }
      void set_temp_down_pin(uint8_t pin) { this->button_pins_.temp_down = (gpio_num_t)pin; }
      void set_filter_pin(uint8_t pin) { this->button_pins_.filter = (gpio_num_t)pin; }

      void setup() override;
      void loop() override;
      void dump_config() override;
      float get_setup_priority() const override { return setup_priority::HARDWARE; }

      /* Fires on the ESPHome main loop, not the reader task. */
      void add_on_state_callback(std::function<void(const vent_panel_state_t &)> &&callback) {
        this->state_callback_.add(std::move(callback));
      }

      vent_panel_state_t get_state() const { return vent_panel_reader_get_state(); }
      bool is_online() const { return vent_panel_reader_is_online(); }

      void set_fan_level(vent_fan_level_enum_t level) { vent_button_control_move_fan_to(level); }
      void set_temp_level(vent_temp_level_enum_t level) { vent_button_control_move_temp_to(level); }
      void press(vent_button_enum_t button) { vent_button_control_press(button); }

      void set_trace(bool enabled) { vent_panel_reader_set_trace(enabled); }
      bool get_trace() const { return vent_panel_reader_get_trace(); }

    protected:
      static void on_panel_state_(const vent_panel_state_t *state, void *ctx);

      vent_panel_reader_config_t reader_config_{};
      vent_button_pins_t button_pins_{};
      CallbackManager<void(const vent_panel_state_t &)> state_callback_;
      std::atomic<bool> dirty_{false};
      bool last_online_{false};
      bool first_publish_{true};
      bool last_move_pending_{false};
    };
  } // namespace vent_bridge
} // namespace esphome
