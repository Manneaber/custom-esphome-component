#include "esphome.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Electra.h"
#include <IRrecv.h>
#include <IRutils.h>
#include <EEPROM.h>

const uint16_t kIrLed = 4;
const uint16_t kRecvPin = 14;
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
    EEPROM.begin(8);

    irrecv.enableIRIn(true);

    bool acState = EEPROM.read(0); // 0x00
    uint8_t acTemp = EEPROM.read(1); // 0x01
    uint8_t acFan = EEPROM.read(2); // 0x02
    uint8_t acMode = EEPROM.read(3); // 0x03
    bool acSwingV = EEPROM.read(4); // 0x04
    bool acSwingH = EEPROM.read(5); // 0x05
    bool acLight = EEPROM.read(6); // 0x06

    if (acTemp == 0) {
      acTemp = 25;
    }
    if (acFan == 0) {
      acFan = kElectraAcFanAuto;
    }
    if (acMode == 0) {
      acMode = kElectraAcAuto;
    }

    ac.begin();

    if (acState) {
      ac.on();
    } else {
      ac.off();
      ac.setPower(false);
    }
   
    ac.setTemp(acTemp);
    ac.setFan(acFan);
    ac.setMode(acMode);
    ac.setSwingV(acSwingV);
    ac.setSwingH(acSwingH);
    ac.setLightToggle(false);

    if (acState) {
      switch (acMode) {
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

    switch (acFan) {
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

    if (acSwingV && acSwingH) {
      this->swing_mode = CLIMATE_SWING_BOTH;
    } else if (acSwingV) {
      this->swing_mode = CLIMATE_SWING_VERTICAL;
    } else if (acSwingH) {
      this->swing_mode = CLIMATE_SWING_HORIZONTAL;
    } else {
      this->swing_mode = CLIMATE_SWING_OFF;
    }

    this->target_temperature = acTemp;

    lightSwitch->publish_state(acLight);
    lastLightState = lightSwitch->state;

    this->publish_state();

    ESP_LOGI("custom", "acState: %d", acState);
    ESP_LOGI("custom", "acTemp: %d", acTemp);
    ESP_LOGI("custom", "acFan: %d", acFan);
    ESP_LOGI("custom", "acMode: %d", acMode);
    ESP_LOGI("custom", "acSwingV: %d", acSwingV);
    ESP_LOGI("custom", "acSwingH: %d", acSwingH);
    ESP_LOGI("custom", "acLight: %d", acLight);
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
        EEPROM.write(0, true);

        switch (tempAC.getMode()) {
          case kElectraAcAuto:
          this->mode = CLIMATE_MODE_AUTO;
          EEPROM.write(3, kElectraAcAuto);
          break;
          case kElectraAcCool:
          this->mode = CLIMATE_MODE_COOL;
          EEPROM.write(3, kElectraAcCool);
          break;
          case kElectraAcDry:
          this->mode = CLIMATE_MODE_DRY;
          EEPROM.write(3, kElectraAcDry);
          break;
          case kElectraAcFan:
          this->mode = CLIMATE_MODE_FAN_ONLY;
          EEPROM.write(3, kElectraAcFan);
          break;
        }
      } else {
        this->mode = CLIMATE_MODE_OFF;
        EEPROM.write(0, false);
      }

      switch (tempAC.getFan()) {
        case kElectraAcFanAuto:
        this->fan_mode = CLIMATE_FAN_AUTO;
        EEPROM.write(2, kElectraAcFanAuto);
        break;
        case kElectraAcFanLow:
        this->fan_mode = CLIMATE_FAN_LOW;
        EEPROM.write(2, kElectraAcFanLow);
        break;
        case kElectraAcFanMed:
        this->fan_mode = CLIMATE_FAN_MEDIUM;
        EEPROM.write(2, kElectraAcFanMed);
        break;
        case kElectraAcFanHigh:
        this->fan_mode = CLIMATE_FAN_HIGH;
        EEPROM.write(2, kElectraAcFanHigh);
        break;
      }

      if (tempAC.getSwingV() && tempAC.getSwingH()) {
        this->swing_mode = CLIMATE_SWING_BOTH;
        EEPROM.write(4, true);
        EEPROM.write(5, true);
      } else if (tempAC.getSwingV()) {
        this->swing_mode = CLIMATE_SWING_VERTICAL;
        EEPROM.write(4, true);
        EEPROM.write(5, false);
      } else if (tempAC.getSwingH()) {
        this->swing_mode = CLIMATE_SWING_HORIZONTAL;
        EEPROM.write(4, false);
        EEPROM.write(5, true);
      } else {
        this->swing_mode = CLIMATE_SWING_OFF;
        EEPROM.write(4, false);
        EEPROM.write(5, false);
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
        EEPROM.write(6, lastLightState);

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
          EEPROM.write(0, true);
          EEPROM.write(3, kElectraAcCool);
          break;
        case CLIMATE_MODE_AUTO:
          ac.on();
          ac.setMode(kElectraAcAuto);
          EEPROM.write(0, true);
          EEPROM.write(3, kElectraAcAuto);
          break;
        case CLIMATE_MODE_FAN_ONLY:
          ac.on();
          ac.setMode(kElectraAcFan);
          EEPROM.write(0, true);
          EEPROM.write(3, kElectraAcFan);
          break;
        case CLIMATE_MODE_DRY:
          ac.on();
          ac.setMode(kElectraAcDry);
          EEPROM.write(0, true);
          EEPROM.write(3, kElectraAcDry);
          break;
        case CLIMATE_MODE_OFF:
          ac.off();
          ac.setPower(false);
          EEPROM.write(0, false);
          break;
      }

      this->mode = mode;
    }

    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      ac.setTemp(temp);
      EEPROM.write(1, temp);

      this->target_temperature = temp;
    }

    if (call.get_fan_mode().has_value()) {
      ClimateFanMode fanMode = *call.get_fan_mode();
      switch(fanMode) {
        case CLIMATE_FAN_AUTO:
          ac.setFan(kElectraAcFanAuto);
          EEPROM.write(2, kElectraAcFanAuto);
          break;
        case CLIMATE_FAN_LOW:
          ac.setFan(kElectraAcFanLow);
          EEPROM.write(2, kElectraAcFanLow);
          break;
        case CLIMATE_FAN_MEDIUM:
          ac.setFan(kElectraAcFanMed);
          EEPROM.write(2, kElectraAcFanMed);
          break;
        case CLIMATE_FAN_HIGH:
          ac.setFan(kElectraAcFanHigh);
          EEPROM.write(2, kElectraAcFanHigh);
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
          EEPROM.write(4, false);
          EEPROM.write(5, false);
          break;
        case CLIMATE_SWING_BOTH:
          ac.setSwingV(true);
          ac.setSwingH(true);
          EEPROM.write(4, true);
          EEPROM.write(5, true);
          break;
        case CLIMATE_SWING_VERTICAL:
          ac.setSwingV(true);
          ac.setSwingH(false);
          EEPROM.write(4, true);
          EEPROM.write(5, false);
          break;
        case CLIMATE_SWING_HORIZONTAL:
          ac.setSwingV(false);
          ac.setSwingH(true);
          EEPROM.write(4, false);
          EEPROM.write(5, true);
          break;
      }

      this->swing_mode = swingMode;
    }

    this->publish_state();
    EEPROM.commit();

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
