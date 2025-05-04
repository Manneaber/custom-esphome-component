#include "esphome.h"

class Shift595InputComponent : public Component {
 public:
  Shift595InputComponent(uint8_t ds_pin, uint8_t src_pin, uint8_t rck_pin)
      : ds_pin_(ds_pin), src_pin_(src_pin), rck_pin_(rck_pin) {}

  void setup() override {
    pinMode(ds_pin_, INPUT);
    pinMode(src_pin_, INPUT);
    pinMode(rck_pin_, INPUT);

    last_src_ = digitalRead(src_pin_);
    last_rck_ = digitalRead(rck_pin_);
    shift_reg_ = 0;
    output_reg_ = 0;
  }

  void loop() override {
    int current_src = digitalRead(src_pin_);
    int current_rck = digitalRead(rck_pin_);

    // Rising edge of SRC -> shift in
    if (last_src_ == LOW && current_src == HIGH) {
      shift_reg_ = (shift_reg_ << 1) | digitalRead(ds_pin_);
    }

    // Rising edge of RCK -> latch
    if (last_rck_ == LOW && current_rck == HIGH) {
      output_reg_ = shift_reg_;
      char bin_str[9];
      for (int i = 7; i >= 0; i--) {
        bin_str[7 - i] = (output_reg_ & (1 << i)) ? '1' : '0';
      }
      bin_str[8] = '\0';
      ESP_LOGD("shift595", "Latched value: %s (0x%02X)", bin_str, output_reg_);
    }

    last_src_ = current_src;
    last_rck_ = current_rck;
  }

 protected:
  uint8_t ds_pin_, src_pin_, rck_pin_;
  int last_src_, last_rck_;
  uint8_t shift_reg_;
  uint8_t output_reg_;
};
