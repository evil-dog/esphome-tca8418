#pragma once

#include "esphome/components/key_provider/key_provider.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

#include <cstdlib>
#include <utility>

namespace esphome {
namespace tca8418 {

class TCA8418Listener {
  public:
    virtual void key_pressed(uint8_t key){};
    virtual void key_released(uint8_t key){};
};

struct TCA8418Interrupt {
  volatile bool int_recieved{false};
  bool init{false};
  static void gpio_intr(TCA8418Interrupt *store);
};

class TCA8418Component : public key_provider::KeyProvider, public Component, public i2c::I2CDevice {
  public:
    void setup() override;
    void loop() override;
    void dump_config() override;

    float get_setup_priority() const override { return setup_priority::DATA; }
    void set_int_pin(InternalGPIOPin *pin) { int_pin_ = pin; }

    void register_listener(TCA8418Listener *listener);

  protected:
    uint16_t prev_config_{0};
    InternalGPIOPin *int_pin_;

    TCA8418Interrupt store_;

    std::vector<TCA8418Listener *> listeners_{};
};

}  // namespace tca8418
}  // namespace esphome
