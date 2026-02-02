# PCA9848

Arduino library for the **PCA9848PW** 8-channel I2C multiplexer with voltage level translation.

## Overview

The PCA9848PW is an 8-channel I2C multiplexer (switch) that allows multiple I2C buses to share a single host controller. Unlike single-channel-at-a-time multiplexers, the PCA9848 uses a **bitmask-based control register**: each of the 8 bits controls one channel (0–7) independently, so multiple channels can be enabled simultaneously. It also supports **voltage level translation** (e.g. 0.8 V–3.6 V), making it suitable for mixed-voltage I2C systems.

**Key features:**
- 8-channel I2C multiplexer
- Bitmask control: enable/disable channels 0–7 independently
- Multi-channel support: several channels can be active at once
- Voltage level translation
- 2 address pins (A0, A1) → I2C addresses 0x70–0x73

**Datasheet:** [NXP PCA9848](https://www.nxp.com/products/interfaces/ic-bus/ic-multiplexers-switches/8-channel-i2c-bus-multiplexer-with-voltage-translation:PCA9848)

## Hardware Connection

- **SDA, SCL:** Connect to the host I2C bus (e.g. Raspberry Pi Pico GP4/GP5 or board SDA/SCL).
- **Address pins A0, A1:**  
  - Left floating or pulled low → 0; pulled high → 1.  
  - Base address 0x70 + (A0 ? 1 : 0) + (A1 ? 2 : 0) → 0x70, 0x71, 0x72, or 0x73.
- **Reset pin (RST):** Active LOW; optional. Connect to GPIO for hardware reset if needed.
- **Voltage translation:** VDD and downstream VDD(Vcc) set the logic levels; see datasheet for mixed-voltage I2C configurations.

## Usage Examples

### Basic Single Channel

```cpp
#include <Wire.h>
#include "PCA9848.h"

PCA9848 mux;

void setup() {
  Wire.begin();
  mux.attach(Wire, 0x70);
  mux.setChannels(0x02);  // Enable channel 1
}

void loop() {
  // Use I2C on channel 1...
  mux.disableAll();
}
```

### Multiple Channels Simultaneously

```cpp
mux.setChannels(0x0C);  // Enable channels 2 and 3 (bits 2 and 3)
```

### Selective Enable/Disable

```cpp
mux.enableChannel(5);   // Add channel 5 to enabled channels
mux.disableChannel(2);  // Remove channel 2 from enabled channels
```

### Address Configuration

```cpp
mux.setAddress(true, false);  // A0=HIGH, A1=LOW -> address 0x71
```

## API Reference

### Initialization

| Method | Description |
|--------|-------------|
| `PCA9848()` | Default constructor. |
| `void attach(TwoWire& w, uint8_t addr = I2C_ADDR_DEFAULT)` | Attach to Wire interface and set address. |
| `bool attached() const` | Returns true if Wire is attached. |

### Address Configuration

| Method | Description |
|--------|-------------|
| `void setAddress(uint8_t addr)` | Set I2C address (valid range 0x70–0x73). |
| `void setAddress(bool A0, bool A1)` | Set address from pin states. |
| `uint8_t address() const` | Get current I2C address. |

### Channel Control (Bitmask-based)

| Method | Description |
|--------|-------------|
| `void enableChannel(uint8_t ch)` | Enable single channel 0–7; others unchanged. |
| `void disableChannel(uint8_t ch)` | Disable single channel; others unchanged. |
| `void setChannels(uint8_t mask)` | Set channels directly via bitmask (0x00–0xFF). |
| `void enableChannels(uint8_t mask)` | Enable multiple channels via bitmask. |
| `void disableChannels(uint8_t mask)` | Disable multiple channels via bitmask. |
| `void disableAll()` | Disable all channels (write 0x00). |
| `void enableAll()` | Enable all channels (write 0xFF). |

### Channel Status

| Method | Description |
|--------|-------------|
| `bool readControl(uint8_t &out) const` | Read control register into `out`. Returns true on success. |
| `uint8_t getChannels() const` | Get currently enabled channels bitmask. |
| `bool isChannelEnabled(uint8_t ch) const` | True if channel 0–7 is enabled. |
| `uint8_t getEnabledChannelCount() const` | Number of enabled channels. |

### Status & Diagnostics

| Method | Description |
|--------|-------------|
| `uint8_t getStatus() const` | Last I2C status: 0=success, 1–4=errors. |
| `bool ok() const` | True if last operation succeeded. |
| `uint8_t lastCtrl() const` | Last control byte written successfully. |
| `void printStatus(Stream &s = Serial) const` | Print status to stream. |

**I2C status values:** 0 = success; 1 = data too long; 2 = NACK on address; 3 = NACK on data; 4 = other error.

### Control Register Format

```
Bit:     7    6    5    4    3    2    1    0
Channel: CH7  CH6  CH5  CH4  CH3  CH2  CH1  CH0
Value:   1 = enabled, 0 = disabled
```

### Differences from PCA9547

| Aspect | PCA9547 | PCA9848 |
|--------|---------|---------|
| Control register | Bit3=enable, Bits[2:0]=channel | Bits[7:0]=channel bitmask |
| Multi-channel | No (single channel) | Yes (simultaneous channels) |
| Address pins | 3 (A0, A1, A2) | 2 (A0, A1) |
| Address range | 0x70–0x77 | 0x70–0x73 |
| Channel selection | `enable(ch)` | `setChannels(mask)` or `enableChannel(ch)` |
| Voltage translation | No | Yes (e.g. 0.8 V–3.6 V) |

## License

MIT
