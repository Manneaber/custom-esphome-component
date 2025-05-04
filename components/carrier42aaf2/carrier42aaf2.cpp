#include "carrier42aaf2.h"

#define DS  D1    // GPIO5 - Serial Data In
#define SRC D2    // GPIO4 - Shift Register Clock (SRCLK)
#define RCK D5    // GPIO14 - Storage Register Clock (RCLK / LATCH)

namespace esphome {
namespace carrier42aaf2 {
    static const char *const TAG = "Carrier42AAF.climate";

    // Setup GPIO PINs
    const uint16_t kIrLed = 4;
    const uint16_t kRecvPin = 14;
    const uint16_t kCaptureBufferSize = 1024;
    const uint8_t kTimeout = 50;

    byte shiftRegister = 0;   // Internal shift register
    byte outputRegister = 0;  // Latched output
    
    void Carrier42AAF::setup() {
      ESP_LOGCONFIG(TAG, "Setting up Carrier42AAF2...");
    }


    void Carrier42AAF::byteToBinaryString(uint8_t value, char *buffer) {
      for (int i = 7; i >= 0; i--) {
        buffer[7 - i] = (value & (1 << i)) ? '1' : '0';
      }
      buffer[8] = '\0';  // Null-terminate
    }

    void Carrier42AAF::dump_config() {
      ESP_LOGCONFIG(TAG, "Carrier config");
    }
    
    void Carrier42AAF::loop() {
      static int lastSRC = LOW;
      static int lastRCK = LOW;

      int currentSRC = digitalRead(SRC);
      int currentRCK = digitalRead(RCK);

      // On rising edge of shift clock (SRC)
      if (lastSRC == LOW && currentSRC == HIGH) {
        int bit = digitalRead(DS);
        shiftRegister = (shiftRegister << 1) | (bit & 0x01);
      }

      // On rising edge of latch clock (RCK)
      if (lastRCK == LOW && currentRCK == HIGH) {
        outputRegister = shiftRegister;

        char binStr[9];
        byteToBinaryString(outputRegister, binStr);
        ESP_LOGCONFIG(TAG, "Latched value: %s", binStr);
      }

      lastSRC = currentSRC;
      lastRCK = currentRCK;
    }

    void Carrier42AAF::toggle_light() {}

    void Carrier42AAF::control(const ClimateCall &call) {}

}  // namespace carrier42aaf2
}  // namespace esphome