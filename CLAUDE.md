# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Retrofit of a **1971 FAIR-PLAY gymnasium scoreboard** using modern ESP32 microcontrollers. The system maintains the original full-sized LED displays while adding wireless control.

### System Topology

- **1 Controller ESP32** (master): User interface device that broadcasts scoreboard state
- **2 Scoreboard ESP32s** (slaves): Receive state via ESP-NOW and drive LED displays
- **Communication**: ESP-NOW wireless protocol (controller → scoreboards)

### Hardware Architecture

Each scoreboard contains:
- **8 seven-segment digits** (4×7 LED matrix per digit):
  - Digit 0-1: Minutes (tens, ones)
  - Digit 2-3: Seconds (tens, ones)
  - Digit 4-5: Home score (tens, ones)
  - Digit 6-7: Away score (tens, ones)
- **4 quarter indicator LEDs** (period/quarter markers)
- **Adafruit TLC5947** LED drivers for controlling individual LEDs
- **Full-sized incandescent/LED bulbs** (power consumption is a design constraint)

### Future Plans

- Custom modular PCB design for production
- Expand 4x7 font from digits-only to full ASCII 128-character set

### Project Structure

Two PlatformIO projects share common code:
- **controller/**: Firmware for master controller device
- **scoreboard/**: Firmware for slave display units
- **shared/**: Packet protocol, fonts, and utilities used by both

### Documentation

- **[STAGES.md](STAGES.md)**: Staged development plan from POC to production
- **[CLAUDE.md](CLAUDE.md)**: This file - guidance for Claude Code
- **hardware/bom.md**: Bill of materials and component sourcing

## Build System

This project uses **PlatformIO** with the Arduino framework for ESP32 targets.

### Building Firmware

Build controller firmware:
```bash
pio run -d controller -e esp32dev
```

Build scoreboard firmware:
```bash
pio run -d scoreboard -e esp32dev
```

Build both (from root):
```bash
pio run -d controller -e esp32dev && pio run -d scoreboard -e esp32dev
```

### Uploading to Device

Upload controller:
```bash
pio run -d controller -e esp32dev --target upload
```

Upload scoreboard:
```bash
pio run -d scoreboard -e esp32dev --target upload
```

### Serial Monitor

Monitor controller serial output:
```bash
pio device monitor -d controller -b 115200
```

Monitor scoreboard serial output:
```bash
pio device monitor -d scoreboard -b 115200
```

## Code Quality

### Linting

The project uses cpplint for C++ style checking on shared headers:
```bash
cpplint --recursive --quiet shared
```

Note: CI runs this but allows failures (|| true).

## Architecture

### Communication Protocol

The system uses ESP-NOW for wireless communication between controller and scoreboard(s). The protocol is defined in `shared/Packets.h`:

- **ScoreboardPkt**: Wire format struct with CRC32 validation
  - Fixed 32-byte packed structure
  - Contains: version, device ID, sequence number, timestamp, flags, brightness, score digits (home/away), clock (mm:ss), period
  - Device ID: 0/1 for individual scoreboards, 255 for broadcast to both
  - `period` field: Maps to the 4 quarter indicator LEDs (0=off, 1-4=quarters)
  - CRC32 protection using polynomial 0xEDB88320
  - Functions: `pkt_finalize_crc()` to compute, `pkt_verify_crc()` to validate

**Packet-to-Display Mapping:**
- `clock_mm`, `clock_ss` → Digits 0-3 (MM:SS)
- `home_digits[3]` → Digits 4-5 (HH)
- `away_digits[3]` → Digits 6-7 (AA)
- `period` → 4 quarter indicator LEDs

### Display System

`shared/Font4x7.h` defines the LED matrix font rendering:
- 4×7 pixel glyphs for digits 0-9, blank, and minus sign
- Each glyph is 7 rows × 4 columns (LSB = left pixel)
- Currently supports digits only; future expansion to ASCII 128-character set planned
- Each of the 8 digits on a scoreboard is rendered using this font
- Physical layout: `MM:SS HH:AA` (minutes:seconds home:away)

The TLC5947 LED driver provides 24 PWM channels per chip. Multiple drivers can be daisy-chained to control all LEDs in a scoreboard:
- 8 digits × 28 LEDs/digit = 224 LEDs (approximately 10 TLC5947 chips per scoreboard)
- 4 quarter indicator LEDs
- Brightness control via PWM

### Project Structure

```
controller/
  src/Controller.ino       - Controller firmware (currently stubs)
  platformio.ini           - PlatformIO config for controller

scoreboard/
  src/Scoreboard.ino       - Scoreboard firmware (currently stubs)
  platformio.ini           - PlatformIO config for scoreboard

shared/
  Packets.h                - Wire protocol and CRC implementation
  Font4x7.h                - LED matrix font glyphs
  Consts.h                 - Shared constants (empty)
  Utils.h/cpp              - Shared utilities (empty)

tools/test-simulator/      - Host-side test harness
  main.cpp                 - Tests CRC roundtrip and ASCII rendering
  MakeFile                 - Build simulator on host machine
```

### Development Status

This project follows a staged development approach. See **[STAGES.md](STAGES.md)** for the complete development plan.

**Current Stage: Stage 2 - Single Digit with TLC5947**

**Stage 1 Completed and Hardware Tested:**
- ESP-NOW wireless communication validated
- Controller firmware with individual peer addressing
- 2 scoreboards receiving and processing packets
- Button control toggles LEDs wirelessly
- Hybrid state synchronization (fast sync + heartbeat)
- MAC addresses documented in `Dev_Addresses.txt`
- Packet sequence tracking and dropped packet detection
- Serial debugging on all devices
- **All hardware tests passed (18/18)**
- Verified 10m+ reliable communication range

**Stage 0 Completed:**
- Base PlatformIO project structure for controller and scoreboard
- ESP-NOW packet protocol with CRC32 validation (`shared/Packets.h`)
- 4×7 digit font system for LED matrix rendering (`shared/Font4x7.h`)
- CI pipeline for automated builds and linting
- Test simulator for host-side validation

**Upcoming Stages:**
- Stage 2: Single digit with TLC5947 (CURRENT)
- Stage 3: Full 8-digit display
- Stage 4: Complete feature set (quarters, robustness, configuration)
- Stage 5: Custom PCB and mechanical integration

## Testing

### Simulator Tool

A host-side simulator in `tools/test-simulator/` can be compiled with g++ to test packet CRC and font rendering without hardware:

```bash
cd tools/test-simulator
make
./test-simulator
```

This tests:
- CRC32 calculation and validation
- ASCII rendering of 8-digit displays

## CI/CD

GitHub Actions workflow (`.github/workflows/build.yml`) runs on every push:
1. Builds both controller and scoreboard firmware
2. Runs cpplint on shared headers (non-blocking)
3. Uses PlatformIO caching for faster builds

## Hardware Notes

### ESP32 Configuration

- Target platform: ESP32-DevKit (esp32dev board)
- Monitor baud rate: 115200
- ESP-NOW for wireless communication (2.4GHz)

### LED Driver

- **Adafruit TLC5947**: 24-channel, 12-bit PWM LED driver
- Supports daisy-chaining for controlling large numbers of LEDs
- Interface: SPI-like serial protocol
- Each scoreboard requires ~10 TLC5947 chips (8 digits + quarter LEDs)

### Power Considerations

- Original 1971 scoreboard uses full-sized incandescent or LED bulbs
- High current draw per LED must be accounted for in power supply design
- TLC5947 constant-current outputs can handle up to 120mA per channel
- Total system power budget is a critical design constraint
- Future PCB design must accommodate proper power distribution and thermal management

### Bill of Materials

See `hardware/bom.md` for component lists and sourcing information.
