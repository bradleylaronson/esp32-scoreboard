[![Build firmware](https://github.com/bradleylaronson/esp32-scoreboard/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/bradleylaronson/esp32-scoreboard/actions/workflows/build.yml)

# ESP32 Scoreboard Retrofit

Modern ESP32-based retrofit for a **1971 FAIR-PLAY gymnasium scoreboard**. This project maintains the original full-sized LED displays while adding wireless control via ESP-NOW communication.

## Project Overview

**System Architecture:**
- 1× Controller ESP32 (master) - User interface device
- 2× Scoreboard ESP32s (slaves) - Drive LED displays
- ESP-NOW wireless protocol for communication
- Individual peer addressing with hybrid state synchronization

**Display Hardware:**
- 8 seven-segment digits per scoreboard (4×7 LED matrix)
- 4 quarter indicator LEDs
- Adafruit TLC5947 LED drivers (future stages)

## Current Status: Stage 1 Complete & Hardware Tested ✅

ESP-NOW proof-of-concept successfully implemented and validated on hardware:
- ✅ Wireless communication (1 controller + 2 scoreboards)
- ✅ Hybrid state sync (request/response + heartbeat)
- ✅ Bidirectional ESP-NOW communication
- ✅ Late-joining scoreboard support (<100ms sync)
- ✅ Packet sequence tracking
- ✅ **Enhanced features:** PWM brightness, LED patterns, serial interface
- ✅ Unit tests (all passing)
- ✅ **Hardware tests (28/28 passed)**
- ✅ **10m+ reliable range verified**

**Next:** Stage 2 - Single digit display with TLC5947 LED driver

## Documentation

- **[STAGES.md](STAGES.md)** - Complete development roadmap (Stage 0-5)
- **[FEATURES.md](FEATURES.md)** - Stage 1 enhanced features (brightness, patterns, serial interface)
- **[STAGE1_UPLOAD_GUIDE.md](STAGE1_UPLOAD_GUIDE.md)** - Step-by-step firmware upload instructions
- **[TESTING.md](TESTING.md)** - Comprehensive test procedures
- **[CLAUDE.md](CLAUDE.md)** - AI assistant guidance and project architecture
- **hardware/bom.md** - Bill of materials and component sourcing

## Quick Start

### Prerequisites
- 3× ESP32-DevKit boards
- PlatformIO installed
- Hardware wired per STAGES.md Stage 1

### Build Firmware

```bash
# Build controller
pio run -d controller -e esp32dev

# Build scoreboard
pio run -d scoreboard -e esp32dev
```

### Upload Firmware

```bash
# Upload to controller
pio run -d controller -e esp32dev --target upload

# Upload to scoreboard
pio run -d scoreboard -e esp32dev --target upload
```

See [STAGE1_UPLOAD_GUIDE.md](STAGE1_UPLOAD_GUIDE.md) for complete upload and testing instructions.

## Testing

### Test Results: All Passed ✅

**Stage 1 Hardware Testing Complete:**
- ✅ Basic functionality: 4/4 passed
- ✅ Hybrid state sync: 5/5 passed
- ✅ Reliability & edge cases: 4/4 passed
- ✅ Enhanced features: 10/10 passed
- ✅ Unit tests: 3/3 passed
- ✅ Build tests: 2/2 passed

**Total: 28/28 tests passed**

### Run Unit Tests

```bash
cd tools/test-simulator
make test
```

### Hardware Testing

See [TESTING.md](TESTING.md) for complete test procedures including:
- Basic communication tests
- Hybrid state synchronization tests
- Reliability and edge case tests
- Range testing (verified 10m+ reliable communication)

## Continuous Integration

This repository uses GitHub Actions to:
- Build both controller and scoreboard firmware
- Run unit tests (CRC validation, font rendering)
- Run static analysis and linting on shared code
- Generate firmware binaries on tagged releases

## Key Features

### Hybrid State Synchronization
- **Fast Sync**: Scoreboards request state on boot (~100ms response)
- **Heartbeat**: Controller broadcasts state every 3 seconds
- **Robust**: Handles late-joining scoreboards and offline scenarios

### Enhanced LED Control
- **PWM Brightness**: 0-255 levels with 4 presets (25%, 50%, 75%, 100%)
- **Blink Patterns**: 4 modes (steady, slow 1Hz, fast 4Hz, SOS pattern)
- **Serial Interface**: Full control via commands (ON, OFF, BRIGHTNESS, MODE, STATUS, HELP)
- **Button Controls**: Short press (toggle), Long press (cycle brightness)
- **Synchronized**: All features work across controller + 2 scoreboards

See [FEATURES.md](FEATURES.md) for complete documentation.

### Reliability
- Packet sequence tracking with dropped packet detection
- 200ms button debouncing
- 5-second timeout with automatic fallback
- Comprehensive serial debugging output

## Project Structure

```
controller/          - Controller firmware (master device)
scoreboard/          - Scoreboard firmware (slave devices)
shared/              - Common code (packets, fonts, utilities)
  ├── Packets.h      - ESP-NOW packet protocols with CRC32
  ├── Font4x7.h      - 4×7 LED matrix font glyphs
  └── ...
tools/test-simulator/ - Host-side unit tests
hardware/            - Bill of materials and schematics
```

## Development Timeline

- ✅ **Stage 0**: Base project structure, CI/CD (Complete)
- ✅ **Stage 1**: ESP-NOW proof-of-concept (Complete)
- 🔄 **Stage 2**: Single digit with TLC5947 (In Progress - waiting on hardware)
- ⏳ **Stage 3**: Full 8-digit display
- ⏳ **Stage 4**: Complete feature set
- ⏳ **Stage 5**: Custom PCB and mechanical integration

See [STAGES.md](STAGES.md) for detailed timeline and requirements.

## Contributing

This is a personal project, but feedback and suggestions are welcome via GitHub issues.

## License

[Add license information]

