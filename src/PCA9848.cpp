#include "PCA9848.h"

PCA9848::PCA9848()
  : wire_(nullptr),
    addr_(I2C_ADDR_DEFAULT),
    status_(4),
    last_ctrl_(0x00) {}

void PCA9848::attach(TwoWire& w, uint8_t addr) {
  wire_ = &w;
  setAddress(addr);
}

bool PCA9848::isAddressValid(uint8_t addr) {
  return (addr >= 0x58u && addr <= 0x5Fu) || (addr >= 0x70u && addr <= 0x77u);
}

void PCA9848::setAddress(uint8_t addr) {
  if (!isAddressValid(addr))
    addr_ = I2C_ADDR_DEFAULT;
  else
    addr_ = addr;
}

void PCA9848::setAddress(bool A0, bool A1) {
  addr_ = I2C_ADDR_DEFAULT;
  if (A0) addr_ |= 0x01;
  if (A1) addr_ |= 0x02;
}

uint8_t PCA9848::detect(TwoWire& w, uint8_t* addrs_out, uint8_t max_count) {
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

void PCA9848::enableChannel(uint8_t ch) {
  if (ch > 7) return;
  uint8_t ctrl = 0;
  if (!readControl(ctrl)) { status_ = 4; return; }
  ctrl |= (1u << ch);
  writeControl(ctrl);
}

void PCA9848::disableChannel(uint8_t ch) {
  if (ch > 7) return;
  uint8_t ctrl = 0;
  if (!readControl(ctrl)) { status_ = 4; return; }
  ctrl &= ~(1u << ch);
  writeControl(ctrl);
}

void PCA9848::setChannels(uint8_t mask) {
  writeControl(mask);
}

void PCA9848::enableChannels(uint8_t mask) {
  uint8_t ctrl = 0;
  if (!readControl(ctrl)) { status_ = 4; return; }
  ctrl |= mask;
  writeControl(ctrl);
}

void PCA9848::disableChannels(uint8_t mask) {
  uint8_t ctrl = 0;
  if (!readControl(ctrl)) { status_ = 4; return; }
  ctrl &= ~mask;
  writeControl(ctrl);
}

void PCA9848::disableAll() {
  writeControl(CHANNEL_DISABLED);
}

void PCA9848::enableAll() {
  writeControl(CHANNEL_ALL);
}

bool PCA9848::readControl(uint8_t& out) const {
  if (!wire_) { status_ = 4; return false; }
  const uint8_t n = wire_->requestFrom((uint8_t)addr_, (uint8_t)1);
  if (n != 1) { status_ = 4; return false; }
  out = wire_->read();
  status_ = 0;
  return true;
}

uint8_t PCA9848::getChannels() const {
  uint8_t data = 0;
  if (!readControl(data)) return last_ctrl_;
  return data;
}

bool PCA9848::isChannelEnabled(uint8_t ch) const {
  if (ch > 7) return false;
  uint8_t ctrl = getChannels();
  return (ctrl & (1u << ch)) != 0;
}

uint8_t PCA9848::getEnabledChannelCount() const {
  uint8_t ctrl = getChannels();
  uint8_t n = 0;
  for (uint8_t i = 0; i < 8; ++i)
    if (ctrl & (1u << i)) ++n;
  return n;
}

void PCA9848::printStatus(Stream& s) const {
  s.print(F("PCA9848 I2C status: "));
  s.println(status_);
}

void PCA9848::writeControl(uint8_t ctrl) {
  if (!wire_) { status_ = 4; return; }
  wire_->beginTransmission(addr_);
  wire_->write(ctrl);
  status_ = wire_->endTransmission();
  if (status_ == 0) last_ctrl_ = ctrl;
}
