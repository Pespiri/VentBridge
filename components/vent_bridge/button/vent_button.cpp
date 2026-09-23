#include "vent_button.h"

#include "esphome/core/log.h"

namespace esphome {
  namespace vent_bridge {

    static const char *const TAG = "vent_bridge.button";

    void VentButton::press_action() {
      this->parent_->press(this->button_);
    }

    void VentButton::dump_config() {
      LOG_BUTTON("", "Vent Button", this);
      ESP_LOGCONFIG(TAG, "  Button id: %d", (int)this->button_);
    }
  } // namespace vent_bridge
} // namespace esphome
