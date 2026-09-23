#pragma once

#include "../vent_bridge.h"
#include "esphome/components/select/select.h"
#include "esphome/core/component.h"

namespace esphome {
  namespace vent_bridge {
    enum VentSelectType : uint8_t {
      VENT_SELECT_AIRFLOW,
      VENT_SELECT_AIR_TEMP,
    };

    class VentSelect : public select::Select, public Component {
    public:
      explicit VentSelect(VentSelectType type) : type_(type) {}

      void set_parent(VentBridge *parent) { this->parent_ = parent; }
      void setup() override;
      void dump_config() override;

    protected:
      void control(const std::string &value) override;

      VentBridge *parent_{nullptr};
      const VentSelectType type_;
    };
  } // namespace vent_bridge
} // namespace esphome
