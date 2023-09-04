#include "esphome.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Electra.h"
#include <IRrecv.h>
#include <IRutils.h>


const uint16_t kIrLed = 12;
const uint16_t kRecvPin = 5;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 50;

IRElectraAc ac(kIrLed);
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

class CarrierAC : public PollingComponent, public Climate {
 private:
  Switch *lightSwitch;
  bool lastLightState = false;
 public:
  CarrierAC(Switch *lightSwitch) : PollingComponent(1000) {
    this->lightSwitch = lightSwitch;
  }

  void setup() override {
    irrecv.enableIRIn(true);

    ac.begin();
    ac.off();
    ac.setPower(false);
    ac.setTemp(25);
    ac.setFan(kElectraAcFanAuto);
    ac.setMode(kElectraAcAuto);
    ac.setSwingV(true);
    ac.setSwingH(true);
    ac.setLightToggle(false);

    this->mode = CLIMATE_MODE_OFF;
    this->target_temperature = 25;
    this->fan_mode = CLIMATE_FAN_AUTO;
    this->swing_mode = CLIMATE_SWING_BOTH;
    lightSwitch->publish_state(true);

    this->publish_state();
  }

  climate::ClimateTraits traits() {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supports_two_point_target_temperature(false);
    traits.set_visual_min_temperature(16);
    traits.set_visual_max_temperature(30);
    traits.set_visual_temperature_step(1.0f);

    traits.set_supported_modes({
      CLIMATE_MODE_OFF,
      CLIMATE_MODE_COOL,
      CLIMATE_MODE_FAN_ONLY,
      CLIMATE_MODE_DRY,
      CLIMATE_MODE_AUTO
    });

    traits.set_supported_fan_modes({
      CLIMATE_FAN_AUTO,
      CLIMATE_FAN_LOW,
      CLIMATE_FAN_MEDIUM,
      CLIMATE_FAN_HIGH
    });

    traits.set_supported_swing_modes({
      CLIMATE_SWING_OFF,
      CLIMATE_SWING_BOTH,
      CLIMATE_SWING_VERTICAL,
      CLIMATE_SWING_HORIZONTAL
    });

    return traits;
  }

  void loop() override {
    if (irrecv.decode(&results)) {
      IRElectraAc tempAC(-1);
      tempAC.setRaw(results.state);

      if (tempAC.getPower()) {
        switch (tempAC.getMode()) {
          case kElectraAcAuto:
          this->mode = CLIMATE_MODE_AUTO;
          break;
          case kElectraAcCool:
          this->mode = CLIMATE_MODE_COOL;
          break;
          case kElectraAcDry:
          this->mode = CLIMATE_MODE_DRY;
          break;
          case kElectraAcFan:
          this->mode = CLIMATE_MODE_FAN_ONLY;
          break;
        }
      } else {
        this->mode = CLIMATE_MODE_OFF;
      }

      switch (tempAC.getFan()) {
        case kElectraAcFanAuto:
        this->fan_mode = CLIMATE_FAN_AUTO;
        break;
        case kElectraAcFanLow:
        this->fan_mode = CLIMATE_FAN_LOW;
        break;
        case kElectraAcFanMed:
        this->fan_mode = CLIMATE_FAN_MEDIUM;
        break;
        case kElectraAcFanHigh:
        this->fan_mode = CLIMATE_FAN_HIGH;
        break;
      }

      if (tempAC.getSwingV() && tempAC.getSwingH()) {
        this->swing_mode = CLIMATE_SWING_BOTH;
      } else if (tempAC.getSwingV()) {
        this->swing_mode = CLIMATE_SWING_VERTICAL;
      } else if (tempAC.getSwingH()) {
        this->swing_mode = CLIMATE_SWING_HORIZONTAL;
      } else {
        this->swing_mode = CLIMATE_SWING_OFF;
      }

      if (tempAC.getLightToggle()) {
        lightSwitch->toggle();
        lastLightState = lightSwitch->state;
      }

      this->target_temperature = tempAC.getTemp();
      this->publish_state();
    }
  }

  void update() override {
    if (lightSwitch->state != lastLightState) {
        ac.setLightToggle(true);
        lastLightState = lightSwitch->state;

        irrecv.disableIRIn();
        ac.send();
        irrecv.enableIRIn(true);

        ac.setLightToggle(false);
    }
  }

  void control(const ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      ClimateMode mode = *call.get_mode();
      switch(mode) {
        case CLIMATE_MODE_COOL:
          ac.on();
          ac.setMode(kElectraAcCool);
          break;
        case CLIMATE_MODE_AUTO:
          ac.on();
          ac.setMode(kElectraAcAuto);
          break;
        case CLIMATE_MODE_FAN_ONLY:
          ac.on();
          ac.setMode(kElectraAcFan);
          break;
        case CLIMATE_MODE_DRY:
          ac.on();
          ac.setMode(kElectraAcDry);
          break;
        case CLIMATE_MODE_OFF:
          ac.off();
          ac.setPower(false);
          break;
      }

      this->mode = mode;
    }

    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      ac.setTemp(temp);

      this->target_temperature = temp;
    }

    if (call.get_fan_mode().has_value()) {
      ClimateFanMode fanMode = *call.get_fan_mode();
      switch(fanMode) {
        case CLIMATE_FAN_AUTO:
          ac.setFan(kElectraAcFanAuto);
          break;
        case CLIMATE_FAN_LOW:
          ac.setFan(kElectraAcFanLow);
          break;
        case CLIMATE_FAN_MEDIUM:
          ac.setFan(kElectraAcFanMed);
          break;
        case CLIMATE_FAN_HIGH:
          ac.setFan(kElectraAcFanHigh);
          break;
      }

      this->fan_mode = fanMode;
    }

    if (call.get_swing_mode().has_value()) {
      ClimateSwingMode swingMode = *call.get_swing_mode();
      switch(swingMode) {
        case CLIMATE_SWING_OFF:
          ac.setSwingV(false);
          ac.setSwingH(false);
          break;
        case CLIMATE_SWING_BOTH:
          ac.setSwingV(true);
          ac.setSwingH(true);
          break;
        case CLIMATE_SWING_VERTICAL:
          ac.setSwingV(true);
          ac.setSwingH(false);
          break;
        case CLIMATE_SWING_HORIZONTAL:
          ac.setSwingV(false);
          ac.setSwingH(true);
          break;
      }

      this->swing_mode = swingMode;
    }

    this->publish_state();

    irrecv.disableIRIn();
    ac.send();
    irrecv.enableIRIn(true);
  }
};

class CarrierACLightSwitch : public Component, public Switch {
 public:
  void write_state(bool state) override {
    publish_state(state);
  }
};
