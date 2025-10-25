# Testing Guide

This document describes testing procedures for the ESP32 scoreboard system.

---

## Stage 1: ESP-NOW Communication Testing

### Prerequisites

- 3× ESP32-DevKit boards with Stage 1 firmware flashed
- Controller and scoreboards powered on
- Serial monitors available (optional but recommended)

---

## Test Suite 1: Basic Communication

### Test 1.1: Power-On Initialization

**Objective**: Verify all devices initialize correctly.

**Procedure:**
1. Power up controller via USB
2. Open serial monitor at 115200 baud
3. Observe startup sequence

**Expected Results:**
```
✓ [OK] Button configured on GPIO 15
✓ [OK] LED configured on GPIO 16
✓ [OK] WiFi MAC Address displayed
✓ [OK] ESP-NOW initialized
✓ [OK] ESP-NOW callbacks registered
✓ [OK] Scoreboard 1 peer added
✓ [OK] Scoreboard 2 peer added
✓ LED flashes 3 times
✓ [READY] message displayed
```

**Pass Criteria**: All initialization steps complete without errors.

---

### Test 1.2: Button Control (ON)

**Objective**: Verify button press toggles LEDs ON.

**Procedure:**
1. Ensure all 3 LEDs are OFF
2. Press controller button once
3. Observe all LEDs

**Expected Results:**
```
✓ Controller LED turns ON
✓ Scoreboard 1 LED turns ON
✓ Scoreboard 2 LED turns ON
✓ Serial shows "New LED State: ON"
✓ Serial shows successful packet transmission to both scoreboards
```

**Pass Criteria**: All 3 LEDs turn ON simultaneously.

---

### Test 1.3: Button Control (OFF)

**Objective**: Verify button press toggles LEDs OFF.

**Procedure:**
1. Ensure all 3 LEDs are ON (from Test 1.2)
2. Press controller button once
3. Observe all LEDs

**Expected Results:**
```
✓ Controller LED turns OFF
✓ Scoreboard 1 LED turns OFF
✓ Scoreboard 2 LED turns OFF
✓ Serial shows "New LED State: OFF"
✓ Serial shows successful packet transmission to both scoreboards
```

**Pass Criteria**: All 3 LEDs turn OFF simultaneously.

---

### Test 1.4: Multiple Button Presses

**Objective**: Verify reliable operation over multiple presses.

**Procedure:**
1. Press button 10 times (alternating ON/OFF)
2. Monitor serial output for errors

**Expected Results:**
```
✓ All LEDs toggle correctly each press
✓ No "ERROR: Failed to queue packet" messages
✓ No dropped packets reported
✓ Sequence numbers increment correctly
```

**Pass Criteria**: All 10 button presses work correctly with no errors.

---

## Test Suite 2: Hybrid State Synchronization

### Test 2.1: Fast Sync (State Request/Response)

**Objective**: Verify scoreboard syncs immediately on boot.

**Procedure:**
1. Power up controller
2. Press button to turn LED ON
3. Power cycle one scoreboard
4. Monitor serial output

**Expected Results:**

**Scoreboard:**
```
✓ [BOOT] Sending state request to controller (ID: 0)
✓ [OK] State request sent
✓ Receives packet within 100ms
✓ [SYNCED] State synchronized with controller!
✓ LED turns ON
```

**Controller:**
```
✓ [STATE REQUEST] Received from scoreboard #0
✓ [RESPONSE] Packet queued for transmission
✓ Packet sent to scoreboard MAC | Status: SUCCESS
```

**Pass Criteria**: Scoreboard LED matches controller state within 100ms of boot.

---

### Test 2.2: Heartbeat Periodic Broadcast

**Objective**: Verify controller sends heartbeat every 3 seconds.

**Procedure:**
1. Power up controller with LED OFF
2. Do NOT press button
3. Monitor serial output for 10 seconds

**Expected Results:**
```
✓ [HEARTBEAT] message appears approximately every 3 seconds
✓ Each heartbeat sends packet to both scoreboards
✓ Packet shows current LED state (OFF)
```

**Pass Criteria**: At least 3 heartbeat broadcasts in 10 seconds (every ~3 seconds).

---

### Test 2.3: Heartbeat Fallback Sync

**Objective**: Verify scoreboard syncs from heartbeat if request fails.

**Setup:**
1. Temporarily power off controller
2. Power up scoreboard (will fail to get response)
3. Wait for timeout warning (~5 seconds)
4. Power up controller with LED in ON state

**Procedure:**
1. Observe scoreboard behavior after controller powers up
2. Monitor serial output

**Expected Results:**
```
✓ Scoreboard shows timeout warning after 5 seconds
✓ When controller starts, scoreboard receives heartbeat
✓ Scoreboard LED syncs to ON state within 3 seconds
✓ No errors or crashes
```

**Pass Criteria**: Scoreboard successfully syncs from heartbeat after timeout.

---

### Test 2.4: Offline Controller Timeout

**Objective**: Verify timeout handling when controller is offline.

**Procedure:**
1. Power up scoreboard WITHOUT controller
2. Monitor serial output for 6 seconds

**Expected Results:**
```
✓ [BOOT] Sending state request
✓ After ~5 seconds: [WARNING] State sync timeout - controller may be offline
✓ [INFO] Will sync from next heartbeat or button press
✓ No crashes or error loops
```

**Pass Criteria**: Timeout warning appears after 5 seconds, system remains stable.

---

### Test 2.5: Independent Scoreboard Reboot

**Objective**: Verify scoreboards can be rebooted independently.

**Procedure:**
1. Power up all 3 devices with LED ON
2. Power cycle Scoreboard 1 only
3. Observe sync behavior
4. Power cycle Scoreboard 2 only
5. Observe sync behavior

**Expected Results:**
```
✓ Scoreboard 1 reboots and resyncs to ON state
✓ Scoreboard 2 remains ON (unaffected by Scoreboard 1 reboot)
✓ Scoreboard 2 reboots and resyncs to ON state
✓ Scoreboard 1 remains ON (unaffected by Scoreboard 2 reboot)
✓ Controller continues operating normally
```

**Pass Criteria**: Each scoreboard can independently reboot and resync without affecting the other.

---

## Test Suite 3: Reliability & Edge Cases

### Test 3.1: Rapid Button Presses

**Objective**: Test system under rapid input conditions.

**Procedure:**
1. Press button rapidly 5 times in quick succession
2. Monitor serial output

**Expected Results:**
```
✓ Debounce prevents multiple triggers (200ms debounce)
✓ Each legitimate press toggles state correctly
✓ Some packets may show higher sequence numbers (expected)
✓ No system crashes or lock-ups
```

**Pass Criteria**: System handles rapid presses gracefully with debounce working correctly.

---

### Test 3.2: Packet Loss Detection

**Objective**: Verify dropped packet detection.

**Procedure:**
1. Note current sequence number
2. Press button multiple times
3. Check scoreboard serial output for dropped packet warnings

**Expected Results:**
```
✓ If packets are dropped: [WARNING] Dropped X packet(s)
✓ Dropped packet count increments correctly
✓ System continues operating normally
```

**Pass Criteria**: Dropped packets are detected and reported (if any occur).

---

### Test 3.3: Extended Operation

**Objective**: Verify system stability over extended period.

**Procedure:**
1. Power up all devices
2. Leave running for 30 minutes
3. Press button every 2-3 minutes
4. Monitor for errors or degradation

**Expected Results:**
```
✓ System remains responsive after 30 minutes
✓ No memory leaks or performance degradation
✓ Heartbeat continues broadcasting every 3 seconds
✓ All devices remain synchronized
```

**Pass Criteria**: System operates normally for 30+ minutes without issues.

---

### Test 3.4: Range Testing

**Objective**: Determine effective communication range.

**Procedure:**
1. Power up controller and scoreboards
2. Gradually move scoreboards away from controller
3. Test communication at various distances
4. Note maximum reliable range

**Expected Results:**
```
✓ Communication works reliably at 5m
✓ Communication works reliably at 10m
✓ Communication may work at 20m+ (depending on environment)
✓ Packet loss increases with distance
```

**Pass Criteria**: Reliable communication at minimum 10 meters in typical environment.

---

## Test Suite 4: Unit Tests (Host-Side)

### Test 4.1: Test Simulator Build

**Objective**: Verify test simulator compiles and runs.

**Procedure:**
```bash
cd tools/test-simulator
make clean
make test
```

**Expected Results:**
```
✓ Compiles without errors
✓ CRC roundtrip test passes
✓ ASCII render displays 8 digits
✓ "All tests passed" message displayed
```

**Pass Criteria**: All unit tests pass.

---

### Test 4.2: CRC Validation

**Objective**: Verify CRC32 calculation is correct.

**Verification**: Check test simulator output shows:
```
[OK] CRC roundtrip test passed. CRC=0x1E69AEA7
```

**Pass Criteria**: CRC test passes with valid checksum.

---

### Test 4.3: Font Rendering

**Objective**: Verify 4×7 font glyphs render correctly.

**Verification**: ASCII render shows recognizable digits:
```
..#. .##. ###. .##. .##. #### ..#. .##.
.##. #..# ...# #..# #..# ...# .##. #..#
..#. ...# ...# #..# #..# ..#. ..#. ...#
..#. ..#. .##. .### .##. ..#. ..#. ..#.
..#. .#.. ...# ...# #..# .#.. ..#. .#..
..#. #... ...# ..#. #..# .#.. ..#. #...
.### #### ###. ##.. .##. .#.. .### ####
```

**Pass Criteria**: All 8 digits render correctly and are visually recognizable.

---

## Test Suite 5: Enhanced Features (Stage 1+)

### Test 5.1: Brightness Control via Serial

**Objective**: Verify PWM brightness control works correctly.

**Procedure:**
1. Open serial monitor on controller (115200 baud)
2. Type: `ON`
3. Type: `BRIGHTNESS 255`
4. Observe LEDs at full brightness
5. Type: `BRIGHTNESS 128`
6. Observe LEDs at half brightness
7. Type: `BRIGHTNESS 64`
8. Observe LEDs at quarter brightness

**Expected Results:**
```
✓ All 3 LEDs respond to brightness commands
✓ Brightness change is smooth and immediate
✓ Serial shows "[CMD] Brightness set to X"
✓ Lower brightness values visibly dimmer
```

**Pass Criteria**: Brightness changes visible on all 3 LEDs.

---

### Test 5.2: Brightness Control via Long Press

**Objective**: Verify button long-press cycles brightness.

**Procedure:**
1. Ensure LED is ON
2. Hold button for 1+ seconds (long press)
3. Release and observe brightness change
4. Repeat 3 more times

**Expected Results:**
```
✓ First long press: Changes to next brightness level
✓ Second long press: Changes to next level
✓ Third long press: Changes to next level
✓ Fourth long press: Cycles back to first level
✓ Serial shows "--- Long Press ---" and brightness percentage
```

**Pass Criteria**: 4 brightness levels cycle correctly (25%, 50%, 75%, 100%).

---

### Test 5.3: Slow Blink Pattern

**Objective**: Verify Mode 1 (slow blink) works.

**Procedure:**
1. Type: `ON`
2. Type: `MODE 1`
3. Observe LEDs for 10 seconds
4. Count blink cycles

**Expected Results:**
```
✓ All 3 LEDs blink in sync
✓ Blink rate: approximately 1 Hz (1 cycle per second)
✓ Timing: 500ms ON, 500ms OFF
✓ Should observe ~10 complete cycles in 10 seconds
```

**Pass Criteria**: LEDs blink at 1 Hz, synchronized across all devices.

---

### Test 5.4: Fast Blink Pattern

**Objective**: Verify Mode 2 (fast blink) works.

**Procedure:**
1. Type: `ON`
2. Type: `MODE 2`
3. Observe LEDs for 5 seconds
4. Verify fast blink rate

**Expected Results:**
```
✓ All 3 LEDs blink rapidly in sync
✓ Blink rate: approximately 4 Hz (4 cycles per second)
✓ Timing: 125ms ON, 125ms OFF
✓ Should observe ~20 complete cycles in 5 seconds
```

**Pass Criteria**: LEDs blink at 4 Hz, synchronized across all devices.

---

### Test 5.5: SOS Pattern

**Objective**: Verify Mode 3 (SOS pattern) works correctly.

**Procedure:**
1. Type: `ON`
2. Type: `MODE 3`
3. Observe LEDs for one complete SOS cycle
4. Verify pattern: ... --- ...

**Expected Results:**
```
✓ Pattern starts with 3 short pulses (...)
✓ Followed by 3 long pulses (---)
✓ Followed by 3 short pulses (...)
✓ Then 1 second pause
✓ Pattern repeats
✓ All 3 LEDs synchronized
```

**Pass Criteria**: SOS pattern recognizable and synchronized.

---

### Test 5.6: Serial Commands - Basic

**Objective**: Verify all basic serial commands work.

**Procedure:**
1. Type each command and verify response:
   - `HELP`
   - `STATUS`
   - `ON`
   - `OFF`

**Expected Results:**
```
HELP → Shows command list
STATUS → Shows current state (LED, brightness, mode, sequence)
ON → LED turns on, shows "[CMD] LED ON"
OFF → LED turns off, shows "[CMD] LED OFF"
```

**Pass Criteria**: All commands execute and show appropriate responses.

---

### Test 5.7: Brightness with Blink Patterns

**Objective**: Verify brightness affects blink pattern intensity.

**Procedure:**
1. Type: `ON`
2. Type: `MODE 2` (fast blink)
3. Type: `BRIGHTNESS 255` → Observe
4. Type: `BRIGHTNESS 128` → Observe
5. Type: `BRIGHTNESS 64` → Observe

**Expected Results:**
```
✓ At 255: Blinks at full brightness
✓ At 128: Blinks dimmer
✓ At 64: Blinks much dimmer
✓ Blink rate stays constant (4 Hz)
✓ Only brightness of pulses changes
```

**Pass Criteria**: Brightness affects pulse intensity, not timing.

---

### Test 5.8: State Sync with Enhanced Features

**Objective**: Verify enhanced features sync on scoreboard boot.

**Procedure:**
1. On controller: `ON`, `BRIGHTNESS 128`, `MODE 2`
2. Power cycle one scoreboard
3. Observe scoreboard sync behavior

**Expected Results:**
```
✓ Scoreboard sends state request
✓ Controller responds with full state
✓ Scoreboard LED: ON, half brightness, fast blink
✓ Syncs within 100ms
✓ Matches controller behavior exactly
```

**Pass Criteria**: Scoreboard syncs brightness and mode correctly.

---

### Test 5.9: Mode Persistence

**Objective**: Verify mode persists across LED on/off.

**Procedure:**
1. Type: `ON`, `MODE 2`, `BRIGHTNESS 64`
2. Type: `OFF`
3. Type: `ON`

**Expected Results:**
```
✓ After turning back ON:
  - Mode still 2 (fast blink)
  - Brightness still 64
  - State restored correctly
```

**Pass Criteria**: Mode and brightness persist across LED toggles.

---

### Test 5.10: Invalid Commands

**Objective**: Verify error handling for invalid commands.

**Procedure:**
1. Type: `INVALID`
2. Type: `BRIGHTNESS 300`
3. Type: `MODE 5`

**Expected Results:**
```
INVALID → "[ERROR] Unknown command. Type HELP for commands."
BRIGHTNESS 300 → "[ERROR] Brightness must be 0-255"
MODE 5 → "[ERROR] Mode must be 0-3 (0=steady, 1=slow, 2=fast, 3=SOS)"
```

**Pass Criteria**: All invalid inputs show appropriate error messages.

---

## Test Suite 6: Firmware Build Tests

### Test 6.1: Controller Firmware Build

**Procedure:**
```bash
pio run -d controller -e esp32dev
```

**Expected Results:**
```
✓ Compiles without errors or warnings
✓ SUCCESS message displayed
✓ Flash usage: ~56% (733,901 bytes)
✓ RAM usage: ~13.3% (43,528 bytes)
```

**Pass Criteria**: Clean build with no errors.

---

### Test 5.2: Scoreboard Firmware Build

**Procedure:**
```bash
pio run -d scoreboard -e esp32dev
```

**Expected Results:**
```
✓ Compiles without errors or warnings
✓ SUCCESS message displayed
✓ Flash usage: ~56% (734,069 bytes)
✓ RAM usage: ~13.3% (43,528 bytes)
```

**Pass Criteria**: Clean build with no errors.

---

## Test Results Summary

### Stage 1 Complete Test Matrix

| Test ID | Test Name | Status | Notes |
|---------|-----------|--------|-------|
| 1.1 | Power-On Initialization | ✅ | Passed |
| 1.2 | Button Control (ON) | ✅ | Passed |
| 1.3 | Button Control (OFF) | ✅ | Passed |
| 1.4 | Multiple Button Presses | ✅ | Passed |
| 2.1 | Fast Sync (Request/Response) | ✅ | Passed |
| 2.2 | Heartbeat Periodic Broadcast | ✅ | Passed |
| 2.3 | Heartbeat Fallback Sync | ✅ | Passed |
| 2.4 | Offline Controller Timeout | ✅ | Passed |
| 2.5 | Independent Scoreboard Reboot | ✅ | Passed |
| 3.1 | Rapid Button Presses | ✅ | Passed |
| 3.2 | Packet Loss Detection | ✅ | Passed |
| 3.3 | Extended Operation | ✅ | Passed |
| 3.4 | Range Testing | ✅ | Passed - Reliable at 10m+ |
| 4.1 | Test Simulator Build | ✅ | Passed |
| 4.2 | CRC Validation | ✅ | Passed |
| 4.3 | Font Rendering | ✅ | Passed |
| 5.1 | Controller Build | ✅ | Passed |
| 5.2 | Scoreboard Build | ✅ | Passed |

**All 18 tests passed successfully! Stage 1 hardware validation complete.**

**Legend:**
- ✅ Passed
- ❌ Failed
- ⬜ Not Yet Tested
- ⚠️ Passed with issues

---

## Troubleshooting Test Failures

### Test 2.1 Fails (Fast Sync)

**Symptoms**: Scoreboard doesn't sync within 100ms

**Checks:**
- Verify controller is receiving state requests (check serial output)
- Verify scoreboard is sending state requests (check serial output)
- Check MAC addresses are correct in Dev_Addresses.txt
- Verify ESP-NOW peer registration successful

---

### Test 2.2 Fails (Heartbeat)

**Symptoms**: No heartbeat broadcasts appearing

**Checks:**
- Verify heartbeat interval (should be every 3 seconds)
- Check serial monitor is connected and working
- Verify firmware was flashed successfully
- Look for `[HEARTBEAT]` tag in serial output

---

### Test 3.4 Fails (Range)

**Symptoms**: Communication fails before 10m

**Checks:**
- Test in open area (away from metal/concrete)
- Check for WiFi interference (crowded 2.4GHz)
- Verify ESP32 antennas are not damaged
- Try different orientations
- Check power supply is adequate

---

## Next Stage Testing

Once Stage 1 tests are complete, proceed to:
- **Stage 2**: TLC5947 single digit rendering tests
- **Stage 3**: Full 8-digit display tests
- **Stage 4**: Complete feature set validation

See STAGES.md for details on upcoming test requirements.
