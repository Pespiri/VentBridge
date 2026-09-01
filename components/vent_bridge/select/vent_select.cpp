#include "vent_select.h"

#include "esphome/core/log.h"

namespace esphome {
  namespace vent_bridge {

    static const char *const TAG = "vent_bridge.select";

    void VentSelect::setup() {
      this->parent_->add_on_state_callback([this](const vent_panel_state_t &s) {
        // Option order mirrors the level enums, so index == level, offset by the
        // first selectable level (airflow starts at MIN, air temperature at NONE).
        optional<std::string> option;
        if (this->type_ == VENT_SELECT_AIRFLOW) {
          // Prefer the in-flight target so the value does not walk through the
          // intermediate levels a multi-step move passes over.
          vent_airflow_level_enum_t level = vent_button_control_pending_airflow();
          if (level == AIRFLOW_LEVEL_UNKNOWN) level = s.airflow_level;
          if (level == AIRFLOW_LEVEL_UNKNOWN) return;
          option = this->at((size_t)(level - AIRFLOW_LEVEL_MIN));
        } else {
          vent_air_temp_level_enum_t level = vent_button_control_pending_air_temp();
          if (level == AIR_TEMP_LEVEL_UNKNOWN) level = s.air_temp_level;
          if (level == AIR_TEMP_LEVEL_UNKNOWN) return;
          option = this->at((size_t)level);
        }
        if (option) {
          this->publish_state(*option);
        }
      });
    }

    void VentSelect::control(const std::string &value) {
      auto index = this->index_of(value);
      if (!index.has_value()) return;

      if (this->type_ == VENT_SELECT_AIRFLOW) {
        this->parent_->set_airflow_level((vent_airflow_level_enum_t)(*index + AIRFLOW_LEVEL_MIN));
      } else {
        this->parent_->set_air_temp_level((vent_air_temp_level_enum_t)*index);
      }
    }

    void VentSelect::dump_config() {
      LOG_SELECT("", "Vent Select", this);
      ESP_LOGCONFIG(TAG, "  Type: %s", this->type_ == VENT_SELECT_AIRFLOW ? "airflow" : "air temperature");
    }
  } // namespace vent_bridge
} // namespace esphome
