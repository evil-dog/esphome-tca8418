#include "tca8418.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tca8418 {

static const char *const TAG = "tca8418";

static const uint8_t TCA8418_REGISTER_CFG = 0x01;
static const uint8_t TCA8418_REGISTER_INT_STAT = 0x02;
static const uint8_t TCA8418_REGISTER_KEY_LOC_EC = 0x03;
static const uint8_t TCA8418_REGISTER_KEY_EVENT_A = 0x04;
static const uint8_t TCA8418_REGISTER_KP_GPIO1 = 0x1D;  // Rows (same as bit position)
static const uint8_t TCA8418_REGISTER_KP_GPIO2 = 0x1E;  // Cols 0-7 (same as bit position)
static const uint8_t TCA8418_REGISTER_KP_GPIO3 = 0x1F;  // Cols 8,9 (lowest 2 bits)

static const uint8_t TCA8418_CFG_BIT_AUTO_INCREMENT = 7;
static const uint8_t TCA8418_CFG_BIT_GPI_EVENT_MODE = 6;
static const uint8_t TCA8418_CFG_BIT_OVERFLOW_MODE = 5;
static const uint8_t TCA8418_CFG_BIT_INTERRUPT_CFG = 4;
static const uint8_t TCA8418_CFG_BIT_OVERFLOR_INT_EN = 3;
static const uint8_t TCA8418_CFG_BIT_KEY_LOCK_INT_EN = 2;
static const uint8_t TCA8418_CFG_BIT_GPI_INT_EN = 1;
static const uint8_t TCA8418_CFG_BIT_KEY_INT_EN = 0;

static const uint8_t TCA8418_INT_STATUS_BIT_CAD = 4;
static const uint8_t TCA8418_INT_STATUS_BIT_OVERFLOW = 3;
static const uint8_t TCA8418_INT_STATUS_BIT_KEY_LOCK = 2;
static const uint8_t TCA8418_INT_STATUS_BIT_GPI = 1;
static const uint8_t TCA8418_INT_STATUS_BIT_KEY = 0;

static const uint8_t TCA8418_KEY_LOCK_BIT_LOCK_EN = 6;
static const uint8_t TCA8418_KEY_LOCK_BIT_LOCK2_STATUS = 5;
static const uint8_t TCA8418_KEY_LOCK_BIT_LOCK1_STATUS = 4;
// Bottom 4 bits in this register are the Event count
static const uint8_t TCA8418_EVENT_COUNT_MASK = 0b00001111;

void TCA8418Interrupt::gpio_intr(TCA8418Interrupt *store) { store->int_recieved = true; }

void TCA8418Component::setup(InternalGPIOPin *int_pin) {
  ESP_LOGCONFIG(TAG, "Setting up TCA8418...");
  uint16_t value;

  if (!this->read_byte_16(TCA8418_REGISTER_CCC, &value)) {
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "Configuring TCA8418...");

  int_pin->setup();
  this->int_pin_ = int_pin->to_isr();
  int_pin->attach_interrupt(TCA8418Interrupt::gpio_intr, &this->store_, gpio::INTERRUPT_FALLING_EDGE);
  this->store_.init = true;
  this->store_.int_recieved = false;

  //  Setup CFG register
  //  Setup KP_GPIO{1..3} (0x1D..0x1F) registers

  uint16_t config = 0;
  // clear single-shot bit
  //        0b0xxxxxxxxxxxxxxx
  config |= 0b0000000000000000;

  // setup multiplier
  //        0bx000xxxxxxxxxxxx
//  config |= TCA8418_MULTIPLIER_P0_N1 << 12;

  if (!this->write_byte_16(TCA8418_REGISTER_CCC, config)) {
    this->mark_failed();
    return;
  }

  this->prev_config_ = config;
}

void TCA8418Component::loop() {
  if (this->store_.int_recieved) {
    //Interrupt recieved, read key presses
    //  Read INT_STAT (0x02) register to find out the type of int
    //     If K_INT bit set, then key is in FIFO
    //  Read KEY_LOC_EC (0x03) register lowest 3 bits for count of events in FIFO
    //  Read the KEY_EVENT_A (0x04) register for key event
    //    bit 7: 0 = key released,  1 = key pressed
    //  Repeat read of KEY_EVENT_A register until = 0, 0 means FIFO empty
    //  Reset INT_STAT flag by writting a 1 to the bit
    this->store_.int_recieved = false;
  }
}

void TCA8418Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Setting up TCA8418...");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with TCA8418 failed!");
  }
  LOG_PIN(" INT_PIN: ", int_pin_);
}

void TCA8418Component::register_listener(TCA8418Listener *listener) { this->listeners_.push_back(listener); }

}  // namespace tca8418
}  // namespace esphome
