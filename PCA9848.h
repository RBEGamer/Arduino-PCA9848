#pragma once
#ifndef PCA9848_H
#define PCA9848_H

#include <Arduino.h>
#include <Wire.h>

/**
 * PCA9848PW 8-channel I2C multiplexer with voltage level translation
 *
 * Control register (bitmask):
 *   bits[7:0] = channel enable mask (bit N = channel N, 1=enabled, 0=disabled)
 *   0x00 = all channels disabled
 *   0xFF = all channels enabled
 *   Multiple channels can be enabled simultaneously.
 *
 * Address: 2 pins (A0, A1) can be tied to LOW, HIGH, SCL, or SDA -> 16 devices
 * on the I2C-bus (datasheet Table 4): 7-bit addresses 0x58..0x5F and 0x70..0x77.
 *
 * Notes:
 * - getStatus() returns Wire.endTransmission() status (0=success), not mux state.
 */
class PCA9848
{
  static constexpr uint8_t I2C_ADDR_DEFAULT = 0x70;
  static constexpr uint8_t I2C_ADDR_MIN   = 0x58;  // A1=A0=SCL (Table 4)
  static constexpr uint8_t I2C_ADDR_MAX    = 0x77;  // full 16-address range
  static constexpr uint8_t I2C_DEVICE_COUNT = 16;

  static constexpr uint8_t CHANNEL_DISABLED = 0x00;
  static constexpr uint8_t CHANNEL_ALL     = 0xFF;

  static constexpr uint8_t CHANNEL_MASK    = 0x07;  // channel index 0..7

public:
  PCA9848()
  : wire_(nullptr),
    addr_(I2C_ADDR_DEFAULT),
    status_(4),            // "other error" until first transaction
    last_ctrl_(0x00) {}

  void attach(TwoWire& w, uint8_t addr = I2C_ADDR_DEFAULT) {
    wire_ = &w;
    setAddress(addr);
  }

  bool attached() const { return wire_ != nullptr; }

  /** Returns true if addr is one of the 16 valid PCA9848 7-bit addresses (Table 4). */
  static bool isAddressValid(uint8_t addr) {
    return (addr >= 0x58u && addr <= 0x5Fu) || (addr >= 0x70u && addr <= 0x77u);
  }

  void setAddress(uint8_t addr) {
    if (!isAddressValid(addr))
      addr_ = I2C_ADDR_DEFAULT;
    else
      addr_ = addr;
  }

  /** Set address from A0/A1 pins when tied to LOW/HIGH only (4 of 16 options). */
  void setAddress(bool A0, bool A1) {
    addr_ = I2C_ADDR_DEFAULT;
    if (A0) addr_ |= 0x01;
    if (A1) addr_ |= 0x02;
  }

  uint8_t address() const { return addr_; }

  /**
   * Scan the I2C bus for PCA9848 devices. Probes all 16 valid addresses (Table 4)
   * and reports which respond. Does not modify this instance's address or status.
   * @param w       I2C bus (e.g. Wire)
   * @param addrs_out  Buffer to receive responding 7-bit addresses (may be nullptr if max_count==0)
   * @param max_count  Max number of addresses to write (e.g. 16)
   * @return Number of devices found (and written to addrs_out, up to max_count)
   */
  static uint8_t detect(TwoWire& w, uint8_t* addrs_out, uint8_t max_count) {
    uint8_t n = 0;
    for (uint8_t a = 0x58u; a <= 0x5Fu && n < max_count; ++a) {
      w.beginTransmission(a);
      if (w.endTransmission() == 0) {
        if (addrs_out) addrs_out[n] = a;
        ++n;
      }
    }
    for (uint8_t a = 0x70u; a <= 0x77u && n < max_count; ++a) {
      w.beginTransmission(a);
      if (w.endTransmission() == 0) {
        if (addrs_out) addrs_out[n] = a;
        ++n;
      }
    }
    return n;
  }

  void enableChannel(uint8_t ch) {
    if (ch > 7) return;
    uint8_t ctrl = 0;
    if (!readControl(ctrl)) { status_ = 4; return; }
    ctrl |= (1u << ch);
    writeControl(ctrl);
  }

  void disableChannel(uint8_t ch) {
    if (ch > 7) return;
    uint8_t ctrl = 0;
    if (!readControl(ctrl)) { status_ = 4; return; }
    ctrl &= ~(1u << ch);
    writeControl(ctrl);
  }

  void setChannels(uint8_t mask) {
    writeControl(mask);
  }

  void enableChannels(uint8_t mask) {
    uint8_t ctrl = 0;
    if (!readControl(ctrl)) { status_ = 4; return; }
    ctrl |= mask;
    writeControl(ctrl);
  }

  void disableChannels(uint8_t mask) {
    uint8_t ctrl = 0;
    if (!readControl(ctrl)) { status_ = 4; return; }
    ctrl &= ~mask;
    writeControl(ctrl);
  }

  void disableAll() {
    writeControl(CHANNEL_DISABLED);
  }

  void enableAll() {
    writeControl(CHANNEL_ALL);
  }

  bool readControl(uint8_t &out) const {
    if (!wire_) { status_ = 4; return false; }
    const uint8_t n = wire_->requestFrom((uint8_t)addr_, (uint8_t)1);
    if (n != 1) { status_ = 4; return false; }
    out = wire_->read();
    status_ = 0;
    return true;
  }

  uint8_t getChannels() const {
    uint8_t data = 0;
    if (!readControl(data)) return last_ctrl_;
    return data;
  }

  bool isChannelEnabled(uint8_t ch) const {
    if (ch > 7) return false;
    uint8_t ctrl = getChannels();
    return (ctrl & (1u << ch)) != 0;
  }

  uint8_t getEnabledChannelCount() const {
    uint8_t ctrl = getChannels();
    uint8_t n = 0;
    for (uint8_t i = 0; i < 8; ++i)
      if (ctrl & (1u << i)) ++n;
    return n;
  }

  uint8_t getStatus() const { return status_; }

  bool ok() const { return status_ == 0; }

  uint8_t lastCtrl() const { return last_ctrl_; }

  void printStatus(Stream &s = Serial) const {
    s.print(F("PCA9848 I2C status: "));
    s.println(status_);
  }

private:
  void writeControl(uint8_t ctrl) {
    if (!wire_) { status_ = 4; return; }
    wire_->beginTransmission(addr_);
    wire_->write(ctrl);
    status_ = wire_->endTransmission();
    if (status_ == 0) last_ctrl_ = ctrl;
  }

  TwoWire* wire_;
  uint8_t  addr_;
  mutable uint8_t status_;
  uint8_t  last_ctrl_;
};

#endif // PCA9848_H
