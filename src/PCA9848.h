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
  static constexpr uint8_t I2C_ADDR_MAX   = 0x77;  // full 16-address range
  static constexpr uint8_t I2C_DEVICE_COUNT = 16;

  static constexpr uint8_t CHANNEL_DISABLED = 0x00;
  static constexpr uint8_t CHANNEL_ALL     = 0xFF;

  static constexpr uint8_t CHANNEL_MASK    = 0x07;  // channel index 0..7

public:
  PCA9848();

  void attach(TwoWire& w, uint8_t addr = I2C_ADDR_DEFAULT);
  bool attached() const { return wire_ != nullptr; }

  /** Returns true if addr is one of the 16 valid PCA9848 7-bit addresses (Table 4). */
  static bool isAddressValid(uint8_t addr);

  void setAddress(uint8_t addr);
  /** Set address from A0/A1 pins when tied to LOW/HIGH only (4 of 16 options). */
  void setAddress(bool A0, bool A1);

  uint8_t address() const { return addr_; }

  /**
   * Scan the I2C bus for PCA9848 devices. Probes all 16 valid addresses (Table 4)
   * and reports which respond. Does not modify this instance's address or status.
   */
  static uint8_t detect(TwoWire& w, uint8_t* addrs_out, uint8_t max_count);

  void enableChannel(uint8_t ch);
  void disableChannel(uint8_t ch);
  void setChannels(uint8_t mask);
  void enableChannels(uint8_t mask);
  void disableChannels(uint8_t mask);
  void disableAll();
  void enableAll();

  bool readControl(uint8_t& out) const;
  uint8_t getChannels() const;
  bool isChannelEnabled(uint8_t ch) const;
  uint8_t getEnabledChannelCount() const;

  uint8_t getStatus() const { return status_; }
  bool ok() const { return status_ == 0; }
  uint8_t lastCtrl() const { return last_ctrl_; }

  void printStatus(Stream& s = Serial) const;

private:
  void writeControl(uint8_t ctrl);

  TwoWire* wire_;
  uint8_t  addr_;
  mutable uint8_t status_;
  uint8_t  last_ctrl_;
};

#endif // PCA9848_H
