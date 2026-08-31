#include "vent_trace_switch.h"

#include "esphome/core/log.h"

namespace esphome {
  namespace vent_bridge {

    // Must be named TAG: LOG_SWITCH() expands to it.
    static const char *const TAG = "vent_bridge.switch";

    void VentTraceSwitch::setup() {
      // Tracing always starts off after a reboot; mirror that rather than restoring,
      // so a forgotten toggle cannot flood the log on every boot.
      this->publish_state(this->parent_->get_trace());
    }

    void VentTraceSwitch::write_state(bool state) {
      this->parent_->set_trace(state);
      this->publish_state(state);
    }

    void VentTraceSwitch::dump_config() { LOG_SWITCH("", "Vent Trace", this); }

  } // namespace vent_bridge
} // namespace esphome
