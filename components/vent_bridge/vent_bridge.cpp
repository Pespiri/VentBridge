#include "vent_bridge.h"

#include "esphome/core/log.h"

namespace esphome {
  namespace vent_bridge {

    static const char *const TAG = "vent_bridge";

    /* Seeed XIAO ESP32-C6 antenna path: GPIO14 picks onboard vs u.FL, and only while
     * GPIO3 keeps the RF switch powered. */
    static constexpr vent_antenna_pins_t ANTENNA_PINS = {
      .rf_switch_enable = GPIO_NUM_3,
      .select = GPIO_NUM_14,
    };
    static constexpr bool USE_EXTERNAL_ANTENNA = true;

    void VentBridge::setup() {
      // ESPHome leaves the IDF runtime log level at ERROR; opt these tags in so the
      // driver's ESP_LOGx output reaches `esphome logs` over the network.
      esp_log_level_set("vent_panel_reader", ESP_LOG_DEBUG);
      esp_log_level_set("vent_button_control", ESP_LOG_DEBUG);
      esp_log_level_set("vent_antenna", ESP_LOG_DEBUG);

      if (vent_antenna_select(&ANTENNA_PINS, USE_EXTERNAL_ANTENNA) != ESP_OK) {
        ESP_LOGW(TAG, "antenna select failed");
      }

      this->reader_config_.rx_buffer_size = 1024;
      this->reader_config_.rx_idle_byte_times = 2;
      this->reader_config_.read_timeout_ms = 20;

      if (vent_panel_reader_init(&this->reader_config_) != ESP_OK) {
        this->mark_failed();
        return;
      }
      if (vent_button_control_init(&this->button_pins_) != ESP_OK) {
        this->mark_failed();
        return;
      }

      vent_panel_reader_set_state_callback(&VentBridge::on_panel_state_, this);
      vent_panel_reader_start_task(2);
      vent_button_control_start_task(3);
    }

    /* Reader task context: only flag work, never touch ESPHome entities here. */
    void VentBridge::on_panel_state_(const vent_panel_state_t *state, void *ctx) {
      (void)state;
      static_cast<VentBridge *>(ctx)->dirty_.store(true, std::memory_order_relaxed);
    }

    void VentBridge::loop() {
      bool online = vent_panel_reader_is_online();
      bool online_changed = online != this->last_online_;
      this->last_online_ = online;

      uint32_t resets = vent_panel_reader_filter_reset_count();
      if (resets != this->last_filter_reset_count_) {
        this->last_filter_reset_count_ = resets;
        this->filter_reset_callback_.call();
      }

      // Publish once at startup so entities report offline instead of staying unknown
      // when the panel never responds.
      bool initial = this->first_publish_;
      this->first_publish_ = false;

      // Publishing on both edges shows the target as soon as a move is queued, and
      // re-publishes the real level once it finishes, so a move that fell short
      // still converges to the truth.
      bool move_pending = vent_button_control_pending_airflow() != AIRFLOW_LEVEL_UNKNOWN ||
                          vent_button_control_pending_air_temp() != AIR_TEMP_LEVEL_UNKNOWN;
      bool move_changed = move_pending != this->last_move_pending_;
      this->last_move_pending_ = move_pending;

      if (!this->dirty_.exchange(false, std::memory_order_relaxed) && !online_changed && !initial && !move_changed) {
        return;
      }

      vent_panel_state_t state = vent_panel_reader_get_state();
      this->state_callback_.call(state);
    }

    void VentBridge::dump_config() {
      ESP_LOGCONFIG(TAG, "Vent Bridge:");
      ESP_LOGCONFIG(TAG, "  UART%d  RX:%d TX:%d @ %" PRIu32 " baud", (int)this->reader_config_.uart_port, (int)this->reader_config_.rx_pin, (int)this->reader_config_.tx_pin, this->reader_config_.baud_rate);
      ESP_LOGCONFIG(TAG, "  Buttons  fan:%d/%d temp:%d/%d filter:%d", (int)this->button_pins_.airflow_up, (int)this->button_pins_.airflow_down, (int)this->button_pins_.air_temp_up, (int)this->button_pins_.air_temp_down, (int)this->button_pins_.filter);
      if (this->is_failed()) {
        ESP_LOGE(TAG, "  Setup failed");
      }
    }
  } // namespace vent_bridge
} // namespace esphome
