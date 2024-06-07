#include "tca8418.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tca8418 {

static const char *const TAG = "tca8418";

static const uint8_t TCA8418_REGISTER_CFG             = 0x01;
static const uint8_t TCA8418_REGISTER_INT_STAT        = 0x02;
static const uint8_t TCA8418_REGISTER_KEY_LCK_EC      = 0x03;
static const uint8_t TCA8418_REGISTER_KEY_EVENT_FIFO  = 0x04;
static const uint8_t TCA8418_REGISTER_GPIO_INT_EN1    = 0x1A;
static const uint8_t TCA8418_REGISTER_GPIO_INT_EN2    = 0x1B;
static const uint8_t TCA8418_REGISTER_GPIO_INT_EN3    = 0x1C;
static const uint8_t TCA8418_REGISTER_GPI_EM1         = 0x20;
static const uint8_t TCA8418_REGISTER_GPI_EM2         = 0x21;
static const uint8_t TCA8418_REGISTER_GPI_EM3         = 0x22;

constexpr uint8_t TCA8418_CFG_AUTO_INCREMENT      { 0b10000000 };
constexpr uint8_t TCA8418_CFG_GPI_EVENT_MODE      { 0b01000000 };
constexpr uint8_t TCA8418_CFG_OVERFLOW_MODE       { 0b00100000 };
constexpr uint8_t TCA8418_CFG_INTERRUPT_CFG       { 0b00010000 };
constexpr uint8_t TCA8418_CFG_OVERFLOR_INT_EN     { 0b00001000 };
constexpr uint8_t TCA8418_CFG_KEY_LOCK_INT_EN     { 0b00000100 };
constexpr uint8_t TCA8418_CFG_GPI_INT_EN          { 0b00000010 };
constexpr uint8_t TCA8418_CFG_KEY_INT_EN          { 0b00000001 };

constexpr uint8_t TCA8418_INT_STATUS_BIT_CAD      { 0b00010000 };
constexpr uint8_t TCA8418_INT_STATUS_BIT_OVERFLOW { 0b00001000 };
constexpr uint8_t TCA8418_INT_STATUS_BIT_KEY_LOCK { 0b00000100 };
constexpr uint8_t TCA8418_INT_STATUS_BIT_GPI      { 0b00000010 };
constexpr uint8_t TCA8418_INT_STATUS_BIT_KEY      { 0b00000001 };

// Bottom 4 bits in this register are the Event count
constexpr uint8_t TCA8418_EVENT_COUNT_MASK        { 0b00001111 };

// Key Event Key Status Mask, Bit 7, 0 = key released, 1 = key pressed
constexpr uint8_t TCA8418_KEY_STATUS_MASK         { 0b10000000 };

void TCA8418Interrupt::gpio_intr(TCA8418Interrupt *store) { store->int_received = true; }

void TCA8418Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up TCA8418...");
  uint16_t value;

  if (!this->read_byte(TCA8418_REGISTER_CFG, &value)) {
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "Configuring TCA8418...");

  this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  this->interrupt_pin_->setup();
  this->interrupt_pin_->attach_interrupt(TCA8418Interrupt::gpio_intr, &this->store_, gpio::INTERRUPT_FALLING_EDGE);
  this->store_.init = true;
  this->store_.int_received = false;
  ESP_LOGCONFIG(TAG, "Interrupt Attached.");

  //  Setup CFG register
  uint8_t config = 0;

  // Enable KEY_INT
  config |= TCA8418_CFG_KEY_INT_EN;

  // Set INT pin to reassert after 50us if still pending interrupts
  config |= TCA8418_CFG_INTERRUPT_CFG;

  if (!this->write_byte(TCA8418_REGISTER_CFG, config)) {
    ESP_LOGE(TAG, "Failed to write config register.");
    this->mark_failed();
    return;
  }
  ESP_LOGCONFIG(TAG, "Wrote config register.");

  //  Setup GPIO_INT_EN{1..3} (0x1A..0x1C) registers
  if (!this->write_byte(TCA8418_REGISTER_GPIO_INT_EN1, 0xFF)) {
    ESP_LOGE(TAG, "Failed to write GPIO_INT_EN1 register.");
    this->mark_failed();
    return;
  }
  if (!this->write_byte(TCA8418_REGISTER_GPIO_INT_EN2, 0xFF)) {
    ESP_LOGE(TAG, "Failed to write GPIO_INT_EN2 register.");
    this->mark_failed();
    return;
  }
  if (!this->write_byte(TCA8418_REGISTER_GPIO_INT_EN3, 0x03)) {
    ESP_LOGE(TAG, "Failed to write GPIO_INT_EN3 register.");
    this->mark_failed();
    return;
  }
  ESP_LOGCONFIG(TAG, "Wrote all GPIO_INT_EN registers.");

  //  Setup GPI_EM{1..3} (0x20..0x22) registers
  if (!this->write_byte(TCA8418_REGISTER_GPI_EM1, 0xFF)) {
    ESP_LOGE(TAG, "Failed to write GPI_EM1 register.");
    this->mark_failed();
    return;
  }
  if (!this->write_byte(TCA8418_REGISTER_GPI_EM2, 0xFF)) {
    ESP_LOGE(TAG, "Failed to write GPI_EM2 register.");
    this->mark_failed();
    return;
  }
  if (!this->write_byte(TCA8418_REGISTER_GPI_EM3, 0x03)) {
    ESP_LOGE(TAG, "Failed to write GPI_EM3 register.");
    this->mark_failed();
    return;
  }
  ESP_LOGCONFIG(TAG, "Wrote all GPI_EM registers.");
  ESP_LOGCONFIG(TAG, "Config complete.");

  this->prev_config_ = config;
}

void TCA8418Component::loop() {
  if (this->store_.int_received) {
    //Interrupt recieved, read key presses
    //  Read INT_STAT (0x02) register to find out the type of int
    uint8_t int_stat;
    if (!this->read_byte(TCA8418_REGISTER_INT_STAT, &int_stat)) {
      ESP_LOGW(TAG, "Failed to read INT_STAT register.");
      this->status_set_warning();
      return;
    }
    //     If K_INT bit set, then key is in FIFO
    if (int_stat & TCA8418_INT_STATUS_BIT_KEY) {
      uint8_t key;
      ESP_LOGD(TAG, "Key event interrupt.");
    //  Read the KEY_EVENT_A (0x04) register for key event
    //  Repeat read of KEY_EVENT_A register until = 0, 0 means FIFO empty
      while (this->read_byte(TCA8418_REGISTER_KEY_EVENT_FIFO, &key) && (key != 0)) {
    //    bit 7: 0 = key released,  1 = key pressed
        ESP_LOGD(TAG, "Key event: %d", key);
        if ((key & TCA8418_KEY_STATUS_MASK) == 0) {
          //key released
          for (auto &listener : this->button_listeners_)
            listener->key_released(key);
        } else {
          //key pressed
          for (auto &listener : this->button_listeners_)
            listener->key_pressed(key & ~TCA8418_KEY_STATUS_MASK);
        }
      }
    //  Reset INT_STAT flag by writing a 1 to the bit
    if (!this->write_byte(TCA8418_REGISTER_INT_STAT, TCA8418_INT_STATUS_BIT_KEY)) {
      ESP_LOGW(TAG, "Failed to clear interrupt status bit.");
      this->status_set_warning();
      return;
    }
    } else {
      // unknown or unsupported interrupt.
      ESP_LOGW(TAG, "Unknown or unsupported interrupt detected.");
      if (!this->write_byte(TCA8418_REGISTER_INT_STAT, 0xFF)) {
        ESP_LOGW(TAG, "Failed to clear all interrupt stats bits.");
        this->status_set_warning();
        return;
      }
    }
    this->store_.int_received = false;
  }
}

void TCA8418Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Setting up TCA8418...");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with TCA8418 failed!");
  }
  LOG_PIN(" INTERRUPT_PIN: ", this->interrupt_pin_);
}

void TCA8418Component::register_listener(TCA8418Listener *listener) { this->button_listeners_.push_back(listener); }

}  // namespace tca8418
}  // namespace esphome
