# TCA8418 Keypad & GPIO for ESPHome
ESPHome component for the TCA8418 I2C keypad and GPIO chip.

## Note

This component is setup for my specific needs. It does not fully implement all the features of the TCA8418.

The current use is for all pins to be setup as GPI pins with pullups, pin change interrupts enabled, and all pins enabled for key events. The INT pin is setup for only the key events to trigger. This gives you key events for 18 buttons.

I may implement more features in the future. But in the mean time, pull requests welcome.

## Description

This component is based on the Matrix Keypad component. You configure the chip with the i2c address and an interrupt pin. You then configure binary_sensors for each key. Valid keys are 97 through 114.

## Schematic

```
                   I2C                    Row/Col Pins     Switches
┌──────────┐                 ┌─────────┐
│          │<----- SDA ----->│         │<----- R0 -----> |  SW97   | <-- GND
│  ESP32/  │<----- SDC ----->│ TCA8418 │<----- R1 -----> |  SW98   | <-- GND
│  ESP8266 │<----- GND ----->│         │<----- C0 -----> |  SW105  | <-- GND
│          │                 │         │<----- C1 -----> |  SW106  | <-- GND
└──────────┘                 └─────────┘
```

## Installation

You can install this component with [ESPHome external components feature](https://esphome.io/components/external_components.html) like this:

```yaml
external_components:
  - source: github://evil-dog/esphome-tca8418@main

i2c:
  sda: GPIO21
  sdc: GPIO22

# the interrupt_pin is required.
tca8418:
  - address: 0x34
    interrupt_pin: GPIO13

# key must be between 97 and 114 (inclusive).
binary_sensor:
  - platform: tca8418
    id: key97
    key: 97
    name: Key 97
  - platform: tca8418
    id: key105
    key: 105
    name: Key 105
```
