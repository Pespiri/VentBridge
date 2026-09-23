#pragma once

#include "../vent_bridge.h"
#include "esphome/components/button/button.h"
#include "esphome/core/component.h"

namespace esphome {
  namespace vent_bridge {
    class VentButton : public button::Button, public Component {
    public:
      explicit VentButton(vent_button_enum_t button) : button_(button) {}

      void set_parent(VentBridge *parent) { this->parent_ = parent; }
      void dump_config() override;

    protected:
      void press_action() override;

      VentBridge *parent_{nullptr};
      const vent_button_enum_t button_;
    };
  } // namespace vent_bridge
} // namespace esphome
