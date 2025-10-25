# Stage 1 Enhanced Features

This document describes the enhanced features added to Stage 1 beyond the basic ESP-NOW proof-of-concept.

---

## Overview

Stage 1 now includes:
- ✅ PWM brightness control (0-255 levels)
- ✅ Four LED blink patterns
- ✅ Serial command interface
- ✅ Enhanced button controls (short/long press)
- ✅ Synchronized behavior across all 3 devices

---

## Feature 1: Brightness Control

### Description
Control LED brightness using PWM (Pulse Width Modulation) from 0 (off) to 255 (maximum brightness).

### Predefined Levels
- **25%**: 64
- **50%**: 128
- **75%**: 192
- **100%**: 255 (default)

### Usage

**Via Serial Commands:**
```
BRIGHTNESS 64    (Set to 25%)
BRIGHTNESS 128   (Set to 50%)
BRIGHTNESS 192   (Set to 75%)
BRIGHTNESS 255   (Set to 100%)
```

**Via Long-Press Button:**
- Hold button for 1+ seconds
- Cycles through: 25% → 50% → 75% → 100% → repeat
- Serial output shows current level

**Example Output:**
```
--- Long Press ---
Brightness: 192 (75%)
Packet #12 (LED ON, Brightness 192, Mode 0) queued for transmission to both scoreboards
```

### Technical Details
- Uses `analogWrite()` for PWM control on GPIO 16
- Brightness applied to all LEDs (controller + 2 scoreboards)
- Brightness persists across LED on/off toggles
- Works with all blink patterns

---

## Feature 2: LED Blink Patterns

### Available Patterns

#### Mode 0: Steady (Default)
- LED remains solid at current brightness
- No blinking

#### Mode 1: Slow Blink
- **Rate**: 1 Hz (1 cycle per second)
- **Timing**: 500ms ON, 500ms OFF
- **Use case**: Gentle attention indicator

#### Mode 2: Fast Blink
- **Rate**: 4 Hz (4 cycles per second)
- **Timing**: 125ms ON, 125ms OFF
- **Use case**: Urgent notification

#### Mode 3: SOS Pattern
- **Pattern**: ... --- ... (International distress signal)
- **Timing**:
  - 3 short pulses (200ms each)
  - 3 long pulses (600ms each)
  - 3 short pulses (200ms each)
  - 1000ms pause before repeat
- **Use case**: Emergency signal, demo mode

### Usage

**Via Serial Commands:**
```
MODE 0    (Steady)
MODE 1    (Slow blink)
MODE 2    (Fast blink)
MODE 3    (SOS pattern)
```

**Example Output:**
```
[CMD] Mode set to 2
Packet #15 (LED ON, Brightness 255, Mode 2) queued for transmission to both scoreboards
```

### Technical Details
- Blink patterns synchronized across all 3 devices
- Brightness setting applies to blink patterns
- Pattern state resets when mode changes
- Independent timing for controller vs scoreboards

---

## Feature 3: Serial Command Interface

### Overview
Full control of the scoreboard system via Serial Monitor at 115200 baud.

### Available Commands

#### LED Control
```
ON                  Turn LED on
OFF                 Turn LED off
```

#### Brightness Control
```
BRIGHTNESS <0-255>  Set brightness level
                    Example: BRIGHTNESS 128
```

#### Pattern Control
```
MODE <0-3>          Set blink pattern
                    0 = Steady
                    1 = Slow blink (1 Hz)
                    2 = Fast blink (4 Hz)
                    3 = SOS pattern
```

#### Information
```
STATUS              Show current state
HELP                Show command list
```

### Usage Example

**Serial Monitor Session:**
```
> HELP
Available Commands:
  ON                  - Turn LED on
  OFF                 - Turn LED off
  BRIGHTNESS <0-255>  - Set brightness
  MODE <0-3>          - Set mode (0=steady, 1=slow, 2=fast, 3=SOS)
  STATUS              - Show current status
  HELP                - Show this help

> STATUS
========== Status ==========
LED State: ON
Brightness: 255 (100%)
Mode: Steady
Packet Sequence: 5
===========================

> BRIGHTNESS 64
[CMD] Brightness set to 64
Packet #6 (LED ON, Brightness 64, Mode 0) queued for transmission to both scoreboards

> MODE 2
[CMD] Mode set to 2
Packet #7 (LED ON, Brightness 64, Mode 2) queued for transmission to both scoreboards

> STATUS
========== Status ==========
LED State: ON
Brightness: 64 (25%)
Mode: Fast Blink
Packet Sequence: 8
===========================
```

### Serial Monitor Setup

**PlatformIO:**
```bash
pio device monitor -d controller -b 115200
```

**Arduino IDE:**
1. Tools → Serial Monitor
2. Set baud rate: 115200
3. Set line ending: "Newline" or "Both NL & CR"

### Command Format
- Commands are **case-insensitive** (ON, on, On all work)
- Whitespace is trimmed automatically
- Invalid commands show error message
- Commands take effect immediately

---

## Feature 4: Enhanced Button Controls

### Button Behaviors

#### Short Press (< 1 second)
- **Action**: Toggle LED on/off
- **Visual**: LED changes state immediately
- **Serial Output**:
  ```
  --- Button Press ---
  New LED State: ON
  ```

#### Long Press (≥ 1 second)
- **Action**: Cycle brightness levels
- **Sequence**: 25% → 50% → 75% → 100% → repeat
- **Visual**: LED brightness changes
- **Serial Output**:
  ```
  --- Long Press ---
  Brightness: 128 (50%)
  ```

### Debouncing
- 200ms debounce period prevents accidental double-triggers
- Press duration measured from press to release
- Reliable operation with tactile buttons

---

## Feature 5: Synchronized Multi-Device Behavior

### Synchronization

All enhanced features work across all 3 devices:
- **Controller**: Initiates state changes, displays own LED
- **Scoreboard 1**: Receives and replicates all states
- **Scoreboard 2**: Receives and replicates all states

### State Persistence

Current state includes:
- LED on/off
- Brightness level (0-255)
- Blink mode (0-3)

All state transmitted via:
- Button presses (immediate)
- Serial commands (immediate)
- Heartbeat (every 3 seconds)
- State requests (on scoreboard boot)

### Late-Joining Scoreboards

When a scoreboard powers on:
1. Sends state request to controller
2. Controller responds with current state
3. Scoreboard syncs to brightness and mode
4. All 3 devices now synchronized

**Example:**
```
Controller: LED ON, Brightness 128, Mode 2 (fast blink)
Scoreboard powers on →
Within 100ms: Scoreboard also at 128 brightness, fast blinking
```

---

## Usage Scenarios

### Scenario 1: Brightness Adjustment Demo

**Goal**: Demonstrate brightness control

```
1. Press button (short) → LED ON
2. Type: BRIGHTNESS 255 → Full brightness
3. Type: BRIGHTNESS 128 → Half brightness
4. Type: BRIGHTNESS 64  → Quarter brightness
5. Hold button 1+ sec   → Cycles back to 100%
```

### Scenario 2: Emergency Signal

**Goal**: Activate SOS pattern

```
1. Type: ON
2. Type: MODE 3
3. Observe SOS pattern on all 3 LEDs
4. Type: MODE 0  → Return to steady
```

### Scenario 3: Attention Indicator

**Goal**: Create blinking notification

```
1. Type: ON
2. Type: BRIGHTNESS 192
3. Type: MODE 1  → Slow blink at 75%
```

### Scenario 4: Range Testing

**Goal**: Test sync at distance with visible brightness

```
1. Type: BRIGHTNESS 255  → Maximum brightness
2. Type: MODE 2          → Fast blink (easy to see)
3. Move scoreboards farther from controller
4. Verify blink pattern stays synchronized
```

---

## Packet Structure

### Stage1Packet Format

```c
struct Stage1Packet {
  uint8_t ledState;     // 0 = OFF, 1 = ON
  uint8_t brightness;   // 0-255 PWM brightness
  uint8_t mode;         // 0=steady, 1=slow, 2=fast, 3=SOS
  uint8_t reserved;     // Padding for alignment
  uint32_t sequence;    // Packet sequence number
};
```

**Size**: 8 bytes (packed structure)

### Backward Compatibility

Enhanced Stage1Packet maintains:
- Original `ledState` and `sequence` fields
- Adds brightness and mode control
- All scoreboards must use matching firmware

---

## Performance Characteristics

### Latency
- **Button to LED**: < 50ms
- **Serial command to LED**: < 100ms
- **State request to sync**: < 100ms
- **Heartbeat interval**: 3 seconds

### Power Consumption
- **At 25% brightness**: ~25% of full power
- **At 50% brightness**: ~50% of full power
- **Blink patterns**: Average 50% duty cycle (modes 1-2)
- **SOS pattern**: Complex duty cycle, approximately 40%

### Resource Usage
- **Controller Flash**: 743,613 bytes (56.7%)
- **Controller RAM**: 43,704 bytes (13.3%)
- **Scoreboard Flash**: 741,221 bytes (56.6%)
- **Scoreboard RAM**: 43,712 bytes (13.3%)

---

## Troubleshooting

### Problem: Commands don't work

**Check:**
- Serial monitor connected to controller (not scoreboard)
- Baud rate set to 115200
- Line ending set to "Newline" or "Both NL & CR"
- Type `HELP` to verify communication

### Problem: Brightness doesn't change

**Check:**
- LED is ON (brightness only applies when LED is on)
- Type `STATUS` to see current brightness value
- Try extreme values: `BRIGHTNESS 255` then `BRIGHTNESS 64`

### Problem: Blink pattern not synchronized

**Check:**
- All devices running updated firmware
- State synced via heartbeat (wait 3 seconds)
- Power cycle scoreboards to force state request

### Problem: Long press doesn't work

**Check:**
- Hold button for full 1 second before releasing
- Debounce may delay first press
- Watch serial output for "Long Press" vs "Button Press"

---

## Future Enhancements

Potential additions for Stage 1+:
- [ ] RSSI signal strength monitoring
- [ ] Custom blink pattern designer
- [ ] Brightness ramping (fade in/out)
- [ ] Pattern sequences (cycle through modes)
- [ ] Configuration persistence (EEPROM)
- [ ] Web interface for control

---

## Technical Notes

### PWM Frequency
- ESP32 default PWM frequency: ~5 kHz
- Flicker-free for human perception
- Compatible with most LEDs

### Timing Precision
- Blink patterns use `millis()` for timing
- SOS pattern maintains strict timing
- No drift over extended operation

### Memory Management
- No dynamic allocation
- Stack-based state variables
- Fixed-size packet structures

---

## See Also

- **[STAGE1_UPLOAD_GUIDE.md](STAGE1_UPLOAD_GUIDE.md)** - Upload and setup instructions
- **[TESTING.md](TESTING.md)** - Test procedures
- **[STAGES.md](STAGES.md)** - Development roadmap
- **[README.md](README.md)** - Project overview
