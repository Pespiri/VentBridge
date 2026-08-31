#include "vent_binary_sensor.h"

namespace esphome {
  namespace vent_bridge {
    void VentBinarySensor::setup() {
      if (this->type_ == VENT_BINARY_FILTER_RESET) {
        this->publish_state(false);
        this->parent_->add_on_filter_reset_callback([this]() {
          this->publish_state(true);
          // Momentary: HA needs a visible pulse, on-device triggers fire on the edge.
          this->set_timeout("pulse", 1000, [this]() { this->publish_state(false); });
        });
        return;
      }

      this->parent_->add_on_state_callback([this](const vent_panel_state_t &s) {
        switch (this->type_) {
          case VENT_BINARY_SUMMER:
            this->publish_state(s.summer_on);
            break;
          case VENT_BINARY_NOTIFICATION:
            this->publish_state(s.notification_on);
            break;
          case VENT_BINARY_ONLINE:
            this->publish_state(this->parent_->is_online());
            break;
          case VENT_BINARY_FILTER_RESET:
            break;
        }
      });
    }
  } // namespace vent_bridge
} // namespace esphome
