# Stage 1 - Firmware Upload Guide

This guide walks you through uploading firmware to all three ESP32 boards for Stage 1 testing.

## Prerequisites

- 3× ESP32-DevKit boards
- 3× USB cables
- Hardware wired according to STAGES.md Stage 1 section

## Overview

1. Upload controller (initial version)
2. Upload scoreboard #1
3. Upload scoreboard #2
4. Update controller with MAC addresses
5. Re-upload controller (final version)
6. Test the system

---

## Step 1: Upload Controller (Initial)

### 1.1 Connect Controller ESP32
Connect your first ESP32 board to your computer via USB.

### 1.2 Upload Firmware
```bash
pio run -d controller -e esp32dev --target upload
```

**Expected output:**
- Should see compilation progress
- Upload progress with percentages
- Final: `SUCCESS`

**If upload fails:**
- Make sure no other program is using the serial port
- Try pressing and holding the BOOT button during upload
- Check USB cable and connections

### 1.3 Open Serial Monitor
```bash
pio device monitor -d controller -b 115200
```

Or use keyboard shortcut if available in your terminal.

### 1.4 Record MAC Address

**Expected serial output:**
```
=================================
Stage 1 Controller - ESP-NOW POC
=================================

[OK] Button configured on GPIO 15
[OK] LED configured on GPIO 16
[OK] WiFi MAC Address: AA:BB:CC:DD:EE:FF  <-- COPY THIS!
[OK] ESP-NOW initialized
[OK] Broadcast peer added

[READY] Press button to toggle LEDs
===============================================
```

**ACTION REQUIRED:**
- Copy the MAC address shown (format: AA:BB:CC:DD:EE:FF)
- Save it in a text file as "Controller MAC: AA:BB:CC:DD:EE:FF"
- The LED should flash 3 times, then stay off

### 1.5 Disconnect Controller
- Press `Ctrl+C` to exit the serial monitor
- Disconnect the USB cable
- Set this board aside (label it "Controller" if helpful)

---

## Step 2: Upload Scoreboard #1

### 2.1 Connect Scoreboard #1 ESP32
Connect your second ESP32 board to your computer via USB.

### 2.2 Upload Firmware
```bash
pio run -d scoreboard -e esp32dev --target upload
```

**Expected output:**
- Compilation and upload progress
- Final: `SUCCESS`

### 2.3 Open Serial Monitor
```bash
pio device monitor -d scoreboard -b 115200
```

### 2.4 Record MAC Address

**Expected serial output:**
```
====================================
Stage 1 Scoreboard #0 - ESP-NOW POC
====================================

[OK] LED configured on GPIO 16

*** IMPORTANT: Copy this MAC address ***
*** MAC Address: 11:22:33:44:55:66 ***  <-- COPY THIS!
*** Add this to Controller.ino SCOREBOARD_x_MAC ***

[OK] ESP-NOW initialized
[OK] Receive callback registered

[READY] Waiting for packets from controller...
==============================================
```

**ACTION REQUIRED:**
- Copy the MAC address shown
- Save it as "Scoreboard #1 MAC: 11:22:33:44:55:66"
- The LED should flash 3 times, then stay off

### 2.5 Disconnect Scoreboard #1
- Press `Ctrl+C` to exit the serial monitor
- Disconnect the USB cable
- Set this board aside (label it "Scoreboard #1")

---

## Step 3: Upload Scoreboard #2

### 3.1 Edit Firmware for Scoreboard #2
Before uploading to the second scoreboard, we need to change its ID.

Open the file:
```bash
# Use your preferred editor
nano scoreboard/src/Scoreboard.ino
# or
code scoreboard/src/Scoreboard.ino
```

Find line 27 and change:
```cpp
#define SCOREBOARD_ID 0  // Change to 1 for the second scoreboard
```

To:
```cpp
#define SCOREBOARD_ID 1  // This is scoreboard #2
```

Save the file.

### 3.2 Connect Scoreboard #2 ESP32
Connect your third ESP32 board to your computer via USB.

### 3.3 Upload Firmware
```bash
pio run -d scoreboard -e esp32dev --target upload
```

### 3.4 Open Serial Monitor
```bash
pio device monitor -d scoreboard -b 115200
```

### 3.5 Record MAC Address

**Expected serial output:**
```
====================================
Stage 1 Scoreboard #1 - ESP-NOW POC  <-- Notice #1 instead of #0
====================================

[OK] LED configured on GPIO 16

*** IMPORTANT: Copy this MAC address ***
*** MAC Address: AA:BB:CC:DD:EE:FF ***  <-- COPY THIS!
*** Add this to Controller.ino SCOREBOARD_x_MAC ***

[OK] ESP-NOW initialized
[OK] Receive callback registered

[READY] Waiting for packets from controller...
==============================================
```

**ACTION REQUIRED:**
- Copy the MAC address shown
- Save it as "Scoreboard #2 MAC: AA:BB:CC:DD:EE:FF"
- The LED should flash 3 times, then stay off

### 3.6 Disconnect Scoreboard #2
- Press `Ctrl+C` to exit the serial monitor
- Disconnect the USB cable
- Set this board aside (label it "Scoreboard #2")

### 3.7 Revert Scoreboard Firmware (Optional)
If you want to keep the default as ID 0, change line 27 back:
```cpp
#define SCOREBOARD_ID 0  // Change to 1 for the second scoreboard
```

This way, the default is ID 0 for future uploads.

---

## Step 4: Update Controller with MAC Addresses

### 4.1 Review Your MAC Addresses
You should now have three MAC addresses saved:
- Controller MAC: XX:XX:XX:XX:XX:XX
- Scoreboard #1 MAC: YY:YY:YY:YY:YY:YY
- Scoreboard #2 MAC: ZZ:ZZ:ZZ:ZZ:ZZ:ZZ

### 4.2 Edit Controller Firmware
Open the controller firmware:
```bash
# Use your preferred editor
nano controller/src/Controller.ino
# or
code controller/src/Controller.ino
```

### 4.3 Update MAC Addresses
Find lines 31-32 and replace the placeholder MAC addresses:

**Before:**
```cpp
uint8_t SCOREBOARD_1_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // REPLACE ME
uint8_t SCOREBOARD_2_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // REPLACE ME
```

**After (example with MAC 11:22:33:44:55:66 and AA:BB:CC:DD:EE:FF):**
```cpp
uint8_t SCOREBOARD_1_MAC[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};  // Scoreboard #1
uint8_t SCOREBOARD_2_MAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};  // Scoreboard #2
```

**Important:** Convert each pair of hex digits to 0xHH format.

### 4.4 Uncomment Peer Registration (Optional)
If you want to send to specific peers instead of broadcast, find lines 113-142 and uncomment the peer registration code.

**Before:**
```cpp
  // Optional: Add specific scoreboard peers (uncomment when MAC addresses are known)
  /*
  memcpy(peerInfo.peer_addr, SCOREBOARD_1_MAC, 6);
  ...
  */
```

**After:**
```cpp
  // Add specific scoreboard peers
  memcpy(peerInfo.peer_addr, SCOREBOARD_1_MAC, 6);
  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.print("[OK] Scoreboard 1 peer added: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", SCOREBOARD_1_MAC[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }

  memcpy(peerInfo.peer_addr, SCOREBOARD_2_MAC, 6);
  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.print("[OK] Scoreboard 2 peer added: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", SCOREBOARD_2_MAC[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  }
```

**Note:** Broadcast mode (default) works fine for Stage 1. You can skip uncommenting this for now.

Save the file.

---

## Step 5: Re-upload Controller

### 5.1 Connect Controller ESP32
Connect your controller ESP32 (the first board) to your computer via USB.

### 5.2 Upload Updated Firmware
```bash
pio run -d controller -e esp32dev --target upload
```

### 5.3 Verify Upload
```bash
pio device monitor -d controller -b 115200
```

**Expected output:**
```
=================================
Stage 1 Controller - ESP-NOW POC
=================================

[OK] Button configured on GPIO 15
[OK] LED configured on GPIO 16
[OK] WiFi MAC Address: XX:XX:XX:XX:XX:XX
[OK] ESP-NOW initialized
[OK] ESP-NOW callbacks registered
[OK] Scoreboard 1 peer added: YY:YY:YY:YY:YY:YY
[OK] Scoreboard 2 peer added: ZZ:ZZ:ZZ:ZZ:ZZ:ZZ

[READY] Press button to toggle LEDs
===============================================
```

### 5.4 Disconnect
Press `Ctrl+C` to exit the monitor.

---

## Step 6: Test the System

### 6.1 Power All Three Boards
You have three options for powering the boards:

**Option A: All via USB (easiest for testing)**
- Connect all 3 boards to USB power (computer or USB power supply)
- Each board's LED should flash 3 times on startup

**Option B: Mixed power**
- Controller via USB (so you can monitor serial output)
- Scoreboards via 5V power supply or USB

**Option C: Battery/External Power**
- All boards powered by external 5V supply via VIN pin
- Controller should also be connected to computer USB for serial monitoring

### 6.2 Open Serial Monitors (Optional)
If you want to see debug output, open serial monitors for all boards.

**Terminal 1 - Controller:**
```bash
pio device monitor -d controller -b 115200
```

**Terminal 2 - Scoreboard #1:**
```bash
pio device monitor -d scoreboard -b 115200
```

**Terminal 3 - Scoreboard #2:**
```bash
pio device monitor -d scoreboard -b 115200
```

**Note:** You may need USB hubs or multiple computers to monitor all three simultaneously.

### 6.3 Press the Button!

Press the button on the controller board.

**Expected behavior:**
1. Controller LED turns ON
2. Scoreboard #1 LED turns ON
3. Scoreboard #2 LED turns ON
4. Serial output shows packet transmission and reception

Press the button again:
1. All 3 LEDs turn OFF

### 6.4 Check Serial Output

**Controller should show:**
```
--- Button Pressed ---
New LED State: ON
Packet #0 (LED ON) queued for transmission to both scoreboards
Packet sent to: YY:YY:YY:YY:YY:YY | Status: SUCCESS
Packet sent to: ZZ:ZZ:ZZ:ZZ:ZZ:ZZ | Status: SUCCESS

[HEARTBEAT] Sending periodic state update
Packet #1 (LED ON) queued for transmission to both scoreboards
Packet sent to: YY:YY:YY:YY:YY:YY | Status: SUCCESS
Packet sent to: ZZ:ZZ:ZZ:ZZ:ZZ:ZZ | Status: SUCCESS
```

**Each Scoreboard should show:**
```
[BOOT] Sending state request to controller (ID: 0)
[OK] State request sent

--- Packet Received ---
From MAC: XX:XX:XX:XX:XX:XX  (controller's MAC)
Sequence: 0
LED State: ON
LED set to: ON
[SYNCED] State synchronized with controller!
Total packets: 1 | Dropped: 0
-----------------------
```

---

## Step 7: Test Hybrid State Synchronization

Stage 1 includes a **hybrid state sync** feature that ensures scoreboards always match the controller's state, even when powered on late.

### 7.1 How It Works

The system uses two sync mechanisms:

1. **Fast Sync (State Request)**: When a scoreboard boots, it sends a request to the controller asking for the current state. The controller responds immediately.

2. **Heartbeat Fallback**: The controller broadcasts the current state every 3 seconds, ensuring late-joining scoreboards eventually sync.

### 7.2 Test Fast Sync (State Request)

1. Power up controller and press button to turn LED **ON**
2. Power up a scoreboard
3. **Expected**: Scoreboard LED turns ON within ~100ms
4. **Serial output should show**:
   - Scoreboard: `[BOOT] Sending state request`
   - Controller: `[STATE REQUEST] Received from scoreboard #0`
   - Controller: `[RESPONSE] Packet #X (LED ON) queued`
   - Scoreboard: `[SYNCED] State synchronized with controller!`

### 7.3 Test Heartbeat Fallback

1. Power up controller with LED in **OFF** state
2. Wait for heartbeat (max 3 seconds)
3. Power up a scoreboard
4. **Expected**: Scoreboard LED syncs to OFF within 3 seconds
5. **Serial output should show**:
   - Controller: `[HEARTBEAT] Sending periodic state update` (every 3 seconds)
   - Scoreboard: Receives heartbeat packet and syncs

### 7.4 Test Offline Controller

1. Power up scoreboard **without** controller running
2. **Expected**: After 5 seconds, scoreboard shows timeout warning
3. **Serial output should show**:
   ```
   [WARNING] State sync timeout - controller may be offline
   [INFO] Will sync from next heartbeat or button press
   ```
4. Power up controller
5. **Expected**: Scoreboard syncs from next heartbeat (within 3 seconds)

---

## Troubleshooting

### Problem: LEDs don't turn on
- Check wiring: LED polarity, resistor connections
- Verify GPIO pins match firmware (Controller Button=GPIO15, Controller LED=GPIO16, Scoreboards=GPIO16)
- Check if LEDs flash 3 times on boot (confirms firmware is running)

### Problem: Controller uploads but scoreboards don't respond
- Ensure all boards are powered on
- Check that ESP-NOW initialized (look for "[OK] ESP-NOW initialized" in serial output)
- Verify broadcast peer was added on controller
- Try moving boards closer together (ESP-NOW range test)

### Problem: Only one scoreboard responds
- Check that both scoreboards are powered on
- Verify both uploaded successfully
- Check serial output on non-responding scoreboard

### Problem: "Packet sent to: ... | Status: FAIL"
- ESP-NOW initialization may have failed
- Try power cycling the controller
- Check WiFi.mode(WIFI_STA) is set correctly

### Problem: Dropped packets reported
- This is normal if you press the button very rapidly
- If it happens on every press, check for interference or range issues
- Try moving boards closer together

### Problem: Upload fails with "serial port not found"
- Check USB cable connection
- Verify correct COM port (try `pio device list`)
- Try a different USB port
- Install or update USB-to-UART drivers (CP210x or CH340)

### Problem: Upload fails with "timed out waiting for packet header"
- Press and hold the BOOT button on ESP32 during upload
- Release after upload starts
- Try a different USB cable
- Reduce baud rate in platformio.ini (add `upload_speed = 115200`)

---

## Success Criteria

### Basic Functionality
✓ All three boards power on and flash LEDs 3 times
✓ Button press toggles all 3 LEDs ON simultaneously
✓ Second button press toggles all 3 LEDs OFF simultaneously
✓ Serial monitor shows successful packet transmission
✓ Serial monitor shows packet reception on scoreboards
✓ No dropped packets under normal button presses
✓ System works reliably over multiple presses

### Hybrid State Sync
✓ Scoreboards send state request on boot
✓ Controller responds to state requests immediately
✓ Late-joining scoreboards sync within 100ms (via request/response)
✓ Heartbeat broadcasts state every 3 seconds
✓ Scoreboards sync from heartbeat if request fails
✓ Timeout warning shown if controller offline for 5+ seconds
✓ Both scoreboards can be rebooted independently and resync correctly

---

## Next Steps

Once Stage 1 is working:
1. Test range by moving scoreboards farther from controller
2. Test reliability over extended period
3. Document any issues or observations
4. Prepare for Stage 2: Single digit with TLC5947

Congratulations! You now have a working ESP-NOW wireless communication system!
