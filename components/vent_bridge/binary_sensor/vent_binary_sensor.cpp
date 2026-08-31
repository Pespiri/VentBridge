#include "vent_binary_sensor.h"

namespace esphome {
  namespace vent_bridge {
    void VentBinarySensor::setup() {
      this->parent_->add_on_state_callback([this](const vent_panel_state_t &s) {
        switch (this->type_) {
          case VENT_BINARY_SUMMER:
            this->publish_state(s.summer_on);
            break;
          case VENT_BINARY_FILTER:
            this->publish_state(s.filter_on);
            break;
          case VENT_BINARY_ONLINE:
            this->publish_state(this->parent_->is_online());
            break;
        }
      });
    }
  } // namespace vent_bridge
} // namespace esphome
