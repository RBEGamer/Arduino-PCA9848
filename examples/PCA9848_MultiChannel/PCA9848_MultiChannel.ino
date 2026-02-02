/*
 * PCA9848 Multi-Channel Example
 *
 * Enables multiple channels simultaneously, demonstrates bitmask operations,
 * and shows selective enable/disable.
 */

#include <Wire.h>
#include "PCA9848.h"

PCA9848 mux;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();
  mux.attach(Wire, 0x70);

  // Enable channels 2 and 3 via bitmask (bits 2 and 3 = 0x0C)
  mux.setChannels(0x0C);
  Serial.println(F("Channels 2 and 3 enabled (mask 0x0C)"));

  uint8_t ch = mux.getChannels();
  Serial.print(F("getChannels() = 0x"));
  Serial.println(ch, HEX);
  Serial.print(F("Enabled count: "));
  Serial.println(mux.getEnabledChannelCount());

  // Add channel 5 to currently enabled channels
  mux.enableChannel(5);
  Serial.println(F("Channel 5 added -> channels 2, 3, 5 enabled"));

  ch = mux.getChannels();
  Serial.print(F("getChannels() = 0x"));
  Serial.println(ch, HEX);

  // Remove channel 2
  mux.disableChannel(2);
  Serial.println(F("Channel 2 removed -> channels 3, 5 enabled"));

  ch = mux.getChannels();
  Serial.print(F("getChannels() = 0x"));
  Serial.println(ch, HEX);

  // Enable all, then disable all
  mux.enableAll();
  Serial.println(F("enableAll() -> mask 0xFF"));

  mux.disableAll();
  Serial.println(F("disableAll() -> mask 0x00"));
}

void loop() {
  delay(1000);
}
