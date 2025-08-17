#include "carrier42aaf.h"

namespace esphome {
namespace carrier42aaf {
    static const char *const TAG = "Carrier42AAF.climate";

    // Setup GPIO PINs
    const uint16_t kIrLed = 4;
    const uint16_t kRecvPin = 14;
    const uint16_t kCaptureBufferSize = 1024;
    const uint8_t kTimeout = 50;

    IRElectraAc ac(kIrLed);
    IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
    
    void Carrier42AAF::setup() {
      ESP_LOGCONFIG(TAG, "Setting up Carrier42AAF...");

      EEPROM.begin(8);

      irrecv.enableIRIn(true);

      // Set up temperature sensor callback if configured
      if (this->current_temperature_sensor_ != nullptr) {
        this->current_temperature_sensor_->add_on_state_callback([this](float state) {
          this->on_temperature_update(state);
        });
      }

      bool acState = EEPROM.read(0); // 0x00
      uint8_t acTemp = EEPROM.read(1); // 0x01
      uint8_t acFan = EEPROM.read(2); // 0x02
      uint8_t acMode = EEPROM.read(3); // 0x03
      bool acSwingV = EEPROM.read(4); // 0x04
      bool acSwingH = EEPROM.read(5); // 0x05

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
  
      this->publish_state();
    }

    void Carrier42AAF::dump_config() {
      ESP_LOGCONFIG(TAG, "Carrier config");
    }
    
    void Carrier42AAF::loop() {
      if (irrecv.decode(&results)) {
        IRElectraAc tempAC(-1);
        tempAC.setRaw(results.state);
        ESP_LOGD(TAG, "Decoded IR signal: %s", results.state);
  
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
  
        this->target_temperature = tempAC.getTemp();
        
        EEPROM.commit();
        this->publish_state();
        irrecv.resume();
      }
    }

    void Carrier42AAF::toggle_light() {
      ac.setLightToggle(true);
      irrecv.disableIRIn();
      ac.send();
      irrecv.enableIRIn(true);

      ac.setLightToggle(false);
    }

    void Carrier42AAF::control(const ClimateCall &call) {
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

  void Carrier42AAF::on_temperature_update(float temperature) {
    if (!isnan(temperature)) {
      this->current_temperature = temperature;
      this->publish_state();
      ESP_LOGD(TAG, "Updated current temperature: %.1f°C", temperature);
    }
  }

}  // namespace carrier42aaf
}  // namespace esphome