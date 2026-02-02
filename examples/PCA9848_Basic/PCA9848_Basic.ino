/*
 * PCA9848 Basic Example
 *
 * Initializes the mux, enables a single channel, optionally scans I2C bus
 * on that channel, then disables all channels.
 */

#include <Wire.h>
#include "PCA9848.h"

PCA9848 mux;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();
  mux.attach(Wire, 0x70);

  if (!mux.attached()) {
    Serial.println(F("Mux not attached"));
    return;
  }

  mux.setChannels(0x02);  // Enable channel 1 only
  if (!mux.ok()) {
    mux.printStatus(Serial);
    return;
  }

  Serial.println(F("Channel 1 enabled. Scanning I2C..."));
  // Simple I2C scan on channel 1
  for (uint8_t addr = 0x08; addr < 0x78; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  Device at 0x"));
      Serial.println(addr, HEX);
    }
  }

  mux.disableAll();
  Serial.println(F("All channels disabled."));
}

void loop() {
  // Idle
  delay(1000);
}
