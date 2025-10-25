# Development Stages

This document outlines the staged development plan for the ESP32 scoreboard system. Each stage builds upon the previous one, allowing for incremental testing and validation.

---

## Stage 0: Base Project Structure ✓
**Status: COMPLETE**

- [x] PlatformIO projects for controller and scoreboard
- [x] Shared code directory structure
- [x] Packet protocol definition with CRC32
- [x] 4×7 font system
- [x] CI/CD pipeline
- [x] Test simulator

---

## Stage 1: ESP-NOW Proof-of-Concept (Current Stage)
**Goal: Validate ESP-NOW communication between 3 ESP32s**

### Hardware Required
- 3× ESP32-DevKit boards
- 1× Push button (controller)
- 3× 5mm LEDs (1 for controller, 2 for scoreboards)
- 3× 220Ω resistors (for LEDs)
- Breadboards and jumper wires

### Wiring
**Controller:**
- Button: GPIO 2 (with internal pull-up)
- Button GND connection
- LED anode → GPIO 4 (through 220Ω resistor)
- LED cathode → GND

**Scoreboards (both identical):**
- LED anode → GPIO 2 (through 220Ω resistor)
- LED cathode → GND

### Firmware Requirements

#### Controller (`controller/src/Controller.ino`)
1. Initialize ESP-NOW as master
2. Register both scoreboard MAC addresses as peers
3. Button handling:
   - Read GPIO 2 with debouncing
   - Toggle state on press
   - Create simple packet with LED state (on/off)
   - Broadcast packet to both scoreboards via ESP-NOW
4. Serial debugging output

#### Scoreboard (`scoreboard/src/Scoreboard.ino`)
1. Initialize ESP-NOW as slave
2. Register callback for incoming packets
3. Parse received packet for LED state
4. Set GPIO 2 HIGH/LOW based on state
5. Serial debugging output showing received packets

### Success Criteria
- [ ] Controller successfully initializes ESP-NOW
- [ ] Both scoreboards pair with controller
- [ ] Button press on controller toggles LEDs on both scoreboards
- [ ] Serial monitor shows packet transmission/reception
- [ ] System works reliably over ~10-20m range

### Notes
- Use simplified packet for this stage (just 1 byte for LED state)
- Can migrate to full ScoreboardPkt structure once basic communication works
- Document MAC addresses of all 3 ESP32s for peer registration

---

## Stage 2: Single Digit with TLC5947
**Goal: Display one 4×7 digit using TLC5947**

### Hardware Required
- Stage 1 hardware (3 ESP32s)
- 2× Adafruit TLC5947 breakout boards per scoreboard (24 channels each = 48 total)
- 28× 5mm LEDs per scoreboard (for one digit)
- Power supply capable of driving LEDs

### Wiring
**Scoreboard:**
- TLC5947 #1 to ESP32:
  - DIN → GPIO 13 (MOSI)
  - CLK → GPIO 14 (SCK)
  - LAT → GPIO 15 (Latch)
  - OE  → GPIO 4 (Output Enable, active LOW)
- TLC5947 #1 DOUT → TLC5947 #2 DIN (daisy-chain)
- TLC5947 #2 shares CLK, LAT, OE with TLC5947 #1
- TLC5947 to LEDs:
  - Board 1, Channels 0-23 → First 24 LEDs of 4×7 digit
  - Board 2, Channels 0-3 → Remaining 4 LEDs of 4×7 digit
  - Board 2, Channels 4-23 → Unused (available for testing)
  - V+ → External 5V supply (not ESP32 5V!)
  - GND → Common ground with ESP32

### Firmware Requirements

#### Controller
1. Build on Stage 1 code
2. Add digit selection (0-9) via button presses or serial input
3. Send digit value in packet
4. Optional: Add second button for digit selection

#### Scoreboard
1. Integrate Adafruit_TLC5947 library
2. Add to `platformio.ini`:
   ```ini
   lib_deps =
       adafruit/Adafruit TLC5947 @ ^1.2.1
   ```
3. Initialize TLC5947 with 2 boards daisy-chained:
   ```cpp
   Adafruit_TLC5947 tlc = Adafruit_TLC5947(2, CLK_PIN, DIN_PIN, LAT_PIN);
   ```
4. Implement font rendering:
   - Read digit from packet
   - Look up glyph in `DIGITS_4x7[]`
   - Map 4×7 pixels (28 LEDs) to TLC5947 channels 0-27
   - Write PWM values (0-4095) for brightness
5. Handle brightness control from packet

### Success Criteria
- [ ] Both TLC5947 boards successfully initialized and daisy-chained
- [ ] Single digit renders correctly (0-9)
- [ ] All 28 LEDs of the digit light up properly
- [ ] Digit updates when controller sends new value
- [ ] Brightness control works via PWM
- [ ] No LED ghosting or flickering

### Notes
- Each 4×7 digit needs 28 LEDs, requiring 2× TLC5947 (24 channels each)
- Channels 0-27 used for one digit, channels 28-47 available for future use
- Test LED current draw and ensure power supply is adequate
- Document channel-to-LED mapping for the digit layout

---

## Stage 3: Full 8-Digit Display
**Goal: Complete scoreboard with 8 digits**

### Hardware Required
- Stage 2 hardware
- Additional TLC5947 boards (9 more per scoreboard for total of 10)
- 224× LEDs per scoreboard (8 digits × 28 LEDs)
- Upgraded power supply for full LED array

### Wiring
- Daisy-chain TLC5947 boards:
  - Board 1 DOUT → Board 2 DIN
  - Board 2 DOUT → Board 3 DIN
  - ... (continue for all 10 boards)
- All boards share CLK, LAT, OE signals
- Separate power rails with adequate current capacity

### Firmware Requirements

#### Controller
1. Implement full ScoreboardPkt protocol
2. Add score tracking (home/away)
3. Add clock/timer functionality (MM:SS)
4. Multiple buttons or serial interface for:
   - Home score +/-
   - Away score +/-
   - Timer start/stop
   - Timer reset
   - Brightness adjustment

#### Scoreboard
1. Expand TLC5947 driver to handle 10 daisy-chained boards
2. Implement full 8-digit rendering:
   - Parse `clock_mm`, `clock_ss`, `home_digits[]`, `away_digits[]`
   - Render all 8 digits from packet
3. Channel mapping for complete display:
   - Digits 0-1: Minutes (channels 0-55)
   - Digits 2-3: Seconds (channels 56-111)
   - Digits 4-5: Home (channels 112-167)
   - Digits 6-7: Away (channels 168-223)

### Success Criteria
- [ ] All 8 digits render correctly
- [ ] Score increments/decrements work
- [ ] Clock counts down correctly
- [ ] Both scoreboards display identical information
- [ ] Brightness adjustment works across all digits
- [ ] System handles rapid updates without packet loss

### Notes
- 10 TLC5947s × 24 channels = 240 channels (224 needed for digits + 4 for quarters)
- May need to optimize packet transmission rate
- Consider adding sequence number validation for dropped packets

---

## Stage 4: Complete Feature Set
**Goal: Production-ready firmware**

### Additions
1. **Quarter/Period Indicators:**
   - Add 4 LEDs for quarter indicators
   - Wire to remaining TLC5947 channels (224-227)
   - Implement `period` field from packet

2. **Enhanced Controller Interface:**
   - LCD display for controller (optional)
   - Rotary encoders for score adjustment
   - Horn/buzzer output
   - Battery/power monitoring

3. **Robustness Features:**
   - Packet retransmission on failure
   - Sequence number tracking
   - Connection status indicators
   - Watchdog timers
   - EEPROM state persistence (scores survive power loss)

4. **Configuration:**
   - Scoreboard ID assignment (0 or 1)
   - Individual vs broadcast mode selection
   - Brightness presets
   - Over-the-air (OTA) firmware updates

### Success Criteria
- [ ] Quarter indicators functional
- [ ] System runs for extended periods without crashes
- [ ] Graceful handling of communication failures
- [ ] User-friendly controller interface
- [ ] Documented configuration procedure

---

## Stage 5: Hardware Integration
**Goal: Custom PCB and mechanical integration**

### PCB Design
1. Controller PCB:
   - ESP32 module
   - Button matrix or rotary encoders
   - Display (LCD/OLED)
   - Power regulation
   - Programming header

2. Scoreboard Driver PCB (modular design):
   - ESP32 module
   - 10× TLC5947 ICs (or equivalent)
   - High-current output connectors for LED arrays
   - Power input with filtering/protection
   - Mounting holes for enclosure

### Mechanical Integration
1. Mount PCBs in existing scoreboard enclosures
2. Connect to original LED bulb sockets
3. Replace incandescent bulbs with LED equivalents if needed
4. Wire management and strain relief
5. Enclosure modifications for ESP32 antennas

### Power Supply Design
1. Calculate total current requirements:
   - 224 LEDs × max current per LED
   - ESP32 current draw
   - TLC5947 current draw
2. Select/design appropriate power supply
3. Thermal management for high-current sections
4. Fusing and protection circuits

### Success Criteria
- [ ] Custom PCBs manufactured and tested
- [ ] System fits in original enclosures
- [ ] Reliable mechanical connections
- [ ] Adequate power delivery with margin
- [ ] Thermal testing under sustained operation
- [ ] Professional appearance and maintainability

---

## Testing & Validation

### Per-Stage Testing
Each stage should include:
1. Unit testing of new functionality
2. Integration testing with previous stages
3. Stress testing (packet loss, power cycling, etc.)
4. Range testing for ESP-NOW communication
5. Documentation of any issues and resolutions

### Final System Testing
1. Full-game simulation (typical basketball/hockey game)
2. Extended runtime testing (8+ hours continuous operation)
3. Multi-day reliability testing
4. Environmental testing (temperature, humidity if applicable)
5. User acceptance testing

---

## Timeline Estimates

- **Stage 1 (POC):** 1-2 days
- **Stage 2 (Single Digit):** 3-5 days
- **Stage 3 (8 Digits):** 1-2 weeks
- **Stage 4 (Features):** 2-3 weeks
- **Stage 5 (Hardware):** 4-8 weeks (includes PCB fab time)

**Total estimated time:** 2-3 months from start to installation

---

## Current Status

**We are at Stage 1** - ready to begin ESP-NOW proof-of-concept implementation.
