#pragma once

#include "../air_conditioner.h"
#include "esphome/components/button/button.h"

namespace esphome {
namespace hisense {
namespace ac {

// Clears the unit's indoor filter cleaning reminder.
class AirConditionFilterResetButton : public button::Button {
 public:
  void set_parent(AirConditioner *parent) { this->parent_ = parent; }

 protected:
  void press_action() override { this->parent_->reset_filter(); }

  AirConditioner *parent_;
};

}  // namespace ac
}  // namespace hisense
}  // namespace esphome
