# haier_legacy

ESPHome external climate component for Haier air conditioners with the older
serial (UART) WiFi module — firmware `R_1.0.00 / e_2.5.14`, Haier Flexis Plus.

This is the original `HaierFlexisPlus.h` "custom" climate class repackaged as a
proper external component, since ESPHome
[removed the `climate: platform: custom` mechanism](https://esphome.io/guides/contributing#a-note-about-custom-components).

Based on the work of Alba Prades, Miguel Angel Lopez and
[Instalator / Haier_WiFi](https://github.com/instalator/Haier_WiFi/).

## Wiring (Wemos D1 mini)

The component talks to the AC over the hardware UART (GPIO1 TX / GPIO3 RX) at
9600 baud, so logging over the same serial port must be disabled
(`logger: baud_rate: 0`).

## Usage

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [haier_legacy]

logger:
  baud_rate: 0   # required: frees the hardware UART for the AC

climate:
  - platform: haier_legacy
    name: "Living Room AC"
```

You can also load it straight from GitHub instead of a local copy:

```yaml
external_components:
  - source: github://Manneaber/custom-esphome-component
    components: [haier_legacy]
```

Supported: HVAC modes (auto/heat/cool/dry/fan-only), fan speeds, vertical and
horizontal swing, target temperature, and current temperature reporting.
