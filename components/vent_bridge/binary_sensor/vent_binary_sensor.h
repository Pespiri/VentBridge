#pragma once

#include "../vent_bridge.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
  namespace vent_bridge {
    enum VentBinarySensorType : uint8_t {
      VENT_BINARY_SUMMER,
      VENT_BINARY_FILTER,
      VENT_BINARY_ONLINE,
    };

    class VentBinarySensor : public binary_sensor::BinarySensor, public Component {
    public:
      explicit VentBinarySensor(VentBinarySensorType type) : type_(type) {}

      void set_parent(VentBridge *parent) { this->parent_ = parent; }
      void setup() override;

    protected:
      VentBridge *parent_{nullptr};
      const VentBinarySensorType type_;
    };
  } // namespace vent_bridge
} // namespace esphome
