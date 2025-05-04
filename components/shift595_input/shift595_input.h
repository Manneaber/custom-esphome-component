#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace shift595_input {

class Shift595Input : public Component {
 public:
  void set_pins(uint8_t ds_pin, uint8_t src_pin, uint8_t rck_pin) {
    ds_pin_ = ds_pin;
    src_pin_ = src_pin;
    rck_pin_ = rck_pin;
  }

  void setup() override {
    pinMode(ds_pin_, INPUT);
    pinMode(src_pin_, INPUT);
    pinMode(rck_pin_, INPUT);
    last_src_ = digitalRead(src_pin_);
    last_rck_ = digitalRead(rck_pin_);
  }

  void loop() override {
    int current_src = digitalRead(src_pin_);
    int current_rck = digitalRead(rck_pin_);

    // Replace LOW/HIGH with 0/1
    if (last_src_ == 0 && current_src == 1) {
      shift_register_ = (shift_register_ << 1) | digitalRead(ds_pin_);
    }

    if (last_rck_ == 0 && current_rck == 1) {
      output_register_ = shift_register_;
      char bin_str[9];
      for (int i = 7; i >= 0; i--) {
        bin_str[7 - i] = (output_register_ & (1 << i)) ? '1' : '0';
      }
      bin_str[8] = '\0';
      ESP_LOGD("shift595_input", "Latched value: %s (0x%02X)", bin_str, output_register_);
    }

    last_src_ = current_src;
    last_rck_ = current_rck;
  }

 protected:
  uint8_t ds_pin_, src_pin_, rck_pin_;
  uint8_t shift_register_ = 0;
  uint8_t output_register_ = 0;
  int last_src_ = 0;  // Use 0 for LOW
  int last_rck_ = 0;  // Use 0 for LOW
};

}  // namespace shift595_input
}  // namespace esphome
