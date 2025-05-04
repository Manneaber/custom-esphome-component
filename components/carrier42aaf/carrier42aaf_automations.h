#pragma once

#include "carrier42aaf.h"
#include "esphome/core/automation.h"

namespace esphome {
namespace carrier42aaf {

template<typename... Ts> class CarrierActionBase : public Action<Ts...> {
 public:
    void set_parent(Carrier42AAF *parent) { this->parent_ = parent; }

 protected:
   Carrier42AAF *parent_;
};

template<typename... Ts> class LightToggleAction : public CarrierActionBase<Ts...> {
 public:
  void play(Ts... x) override { this->parent_->toggle_light(); }
};


}  // namespace carrier42aaf
}  // namespace esphome