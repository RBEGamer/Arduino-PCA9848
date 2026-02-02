/*
 * PCA9848 Address Configuration Example
 *
 * Demonstrates different address configurations via setAddress(uint8_t)
 * and setAddress(bool A0, bool A1). Shows how to use multiple muxes on
 * the same I2C bus.
 *
 * Schematic (two PCA9848 on shared I2C bus, address via A0/A1):
 *
 *     Arduino/ESP          I2C bus              PCA9848 #1 (0x70)    PCA9848 #2 (0x71)
 *     -----------          --------             -----------------    -----------------
 *         SDA ---------------+------------------------ SDA    ------------ SDA
 *         SCL ---------------+------------------------ SCL    ------------ SCL
 *                            |
 *                      A0=L -+- A1=L (Mux1)      A0=H -+- A1=L (Mux2)
 *                      to GND   to GND           to 3V   to GND
 *                            |
 *                          VCC, GND to each mux; Ch0..Ch7 = downstream I2C buses
 */

#include <Wire.h>
#include "PCA9848.h"

PCA9848 mux1;
PCA9848 mux2;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();

  // First mux at base address 0x70 (A0=LOW, A1=LOW)
  mux1.attach(Wire, 0x70);
  Serial.print(F("Mux1 address: 0x"));
  Serial.println(mux1.address(), HEX);

  // Second mux: set address by pin states A0=HIGH, A1=LOW -> 0x71
  mux2.attach(Wire);
  mux2.setAddress(true, false);
  Serial.print(F("Mux2 address (A0=1, A1=0): 0x"));
  Serial.println(mux2.address(), HEX);

  // Or set address directly (0x70-0x73)
  mux2.setAddress(0x72);
  Serial.print(F("Mux2 address set to 0x72: 0x"));
  Serial.println(mux2.address(), HEX);

  // Use mux1 channel 0, mux2 channel 1 on same bus
  mux1.setChannels(0x01);   // Mux1: channel 0
  mux2.setChannels(0x02);   // Mux2: channel 1
  Serial.println(F("Mux1 ch0 and Mux2 ch1 active (different physical buses)"));

  mux1.disableAll();
  mux2.disableAll();
  Serial.println(F("Both muxes disabled."));
}

void loop() {
  delay(1000);
}
