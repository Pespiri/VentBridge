#pragma once

#include "../vent_bridge.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"

namespace esphome {
  namespace vent_bridge {

    class VentTraceSwitch : public switch_::Switch, public Component {
    public:
      void set_parent(VentBridge *parent) { this->parent_ = parent; }
      void setup() override;
      void dump_config() override;

    protected:
      void write_state(bool state) override;

      VentBridge *parent_{nullptr};
    };

  } // namespace vent_bridge
} // namespace esphome
