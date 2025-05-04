#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"

namespace esphome {
namespace carrier42aaf2 {

using namespace esphome::climate;

class Carrier42AAF : public Component, public Climate {
    public:
     void setup() override;
     void loop() override;
     void dump_config() override;
     void toggle_light();
     float get_setup_priority() const override { return setup_priority::DATA; }
     climate::ClimateTraits traits() override {
        auto traits = climate::ClimateTraits();
        traits.set_supports_current_temperature(true);
        traits.set_supports_two_point_target_temperature(false);
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
        traits.set_visual_min_temperature(16);
        traits.set_visual_max_temperature(30);
        traits.set_visual_temperature_step(1.0f);
        return traits;
     }
  
    protected:
     void control(const ClimateCall &call) override;

    private:
     void byteToBinaryString(uint8_t value, char *buffer);
};

}  // namespace carrier42aaf2
}  // namespace esphome