#pragma once

#include "esphome/components/tca8418/tca8418.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace tca8418{

class TCA8418ComponentBinarySensor : public TCA8418Listener, public binary_sensor::BinarySensor {
  public:
    TCA8418ComponentBinarySensor(uint8_t key) : has_key_(true), key_(key){};
    TCA8418ComponentBinarySensor(const char *key) : has_key_(true), key_((uint8_t) key[0]){};

    void key_pressed(uint8_t key) override {
      if (!this->has_key_)
        return;
      if (key == this->key_)
        this->publish_state(true);
    }

    void key_released(uint8_t key) override {
      if (!this->has_key_)
        return;
      if (key == this->key_)
        this->publish_state(true);
    }

  protected:
    bool has_key_;
    uint8_t key_;
};

} // namespace tca8418
} // namespace esphome