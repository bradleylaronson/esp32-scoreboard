/*
 * Stage 1 Controller - ESP-NOW Proof of Concept
 *
 * Hardware:
 * - Button on GPIO 2 (active LOW, using internal pull-up)
 * - LED on GPIO 4 (through 220Ω resistor to GND)
 * - Button press toggles LED state on controller and both scoreboards
 *
 * Setup Instructions:
 * 1. Upload this firmware to controller ESP32
 * 2. Open serial monitor to see MAC address
 * 3. Note the scoreboard MAC addresses from their serial output
 * 4. Update SCOREBOARD_1_MAC and SCOREBOARD_2_MAC below
 * 5. Re-upload firmware
 */

#include <WiFi.h>
#include <esp_now.h>
#include "../../shared/Packets.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// Button GPIO
#define BUTTON_PIN 15  // Changed from 2 - safer pin choice

// LED GPIO
#define LED_PIN 16     // Changed from 4 - safer pin choice

// Scoreboard MAC Addresses
// Updated from Dev_Addresses.txt
// Format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
uint8_t SCOREBOARD_1_MAC[] = {0x78, 0x1C, 0x3C, 0xCB, 0xD7, 0x4C};  // Scoreboard 1
uint8_t SCOREBOARD_2_MAC[] = {0x44, 0x1D, 0x64, 0xF8, 0x26, 0x2C};  // Scoreboard 2

// ============================================================================
// GLOBAL STATE
// ============================================================================

bool ledState = false;           // Current LED state (off/on)
uint8_t brightness = 255;        // Current brightness (0-255)
uint8_t mode = 0;                // Current mode (0=steady, 1=slow_blink, 2=fast_blink, 3=SOS)
uint32_t packetSequence = 0;     // Packet counter

unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_MS = 200;  // 200ms debounce
const unsigned long LONG_PRESS_MS = 1000;  // 1 second for long press

// Heartbeat for periodic state broadcast
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL_MS = 3000;  // 3 seconds

// Brightness levels for cycling
const uint8_t BRIGHTNESS_LEVELS[] = {64, 128, 192, 255};  // 25%, 50%, 75%, 100%
uint8_t brightnessIndex = 3;  // Start at 100%

// Controller LED blink pattern state (matches scoreboard behavior)
unsigned long lastControllerBlinkTime = 0;
bool controllerBlinkState = false;
int controllerSosIndex = 0;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void sendCurrentState(bool isResponse = false);
void processSerialCommand();
void printStatus();
void updateControllerLED();

// ============================================================================
// ESP-NOW CALLBACKS
// ============================================================================

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Packet sent to: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" | Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");
}

void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len) {
  // Check if this is a StateRequest packet
  if (len == sizeof(StateRequest)) {
    StateRequest *request = (StateRequest*)data;

    // Verify magic byte
    if (request->magic == 0xAA) {
      Serial.print("\n[STATE REQUEST] Received from scoreboard #");
      Serial.print(request->scoreboardID);
      Serial.print(" (MAC: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac_addr[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println(")");

      // Send immediate response with current state
      sendCurrentState(true);  // true = responding to request
    }
  }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void sendCurrentState(bool isResponse) {
  Stage1Packet packet;
  packet.ledState = ledState ? 1 : 0;
  packet.brightness = brightness;
  packet.mode = mode;
  packet.reserved = 0;
  packet.sequence = packetSequence++;

  // Send packet to Scoreboard 1
  esp_err_t result1 = esp_now_send(SCOREBOARD_1_MAC,
                                    (uint8_t*)&packet,
                                    sizeof(packet));

  // Send packet to Scoreboard 2
  esp_err_t result2 = esp_now_send(SCOREBOARD_2_MAC,
                                    (uint8_t*)&packet,
                                    sizeof(packet));

  if (result1 == ESP_OK && result2 == ESP_OK) {
    if (isResponse) {
      Serial.print("[RESPONSE] ");
    }
    Serial.print("Packet #");
    Serial.print(packet.sequence);
    Serial.print(" (LED ");
    Serial.print(ledState ? "ON" : "OFF");
    Serial.print(", Brightness ");
    Serial.print(brightness);
    Serial.print(", Mode ");
    Serial.print(mode);
    Serial.println(") queued for transmission to both scoreboards");
  } else {
    if (result1 != ESP_OK) {
      Serial.println("ERROR: Failed to queue packet to Scoreboard 1!");
    }
    if (result2 != ESP_OK) {
      Serial.println("ERROR: Failed to queue packet to Scoreboard 2!");
    }
  }
}

void printStatus() {
  Serial.println("\n========== Status ==========");
  Serial.print("LED State: ");
  Serial.println(ledState ? "ON" : "OFF");
  Serial.print("Brightness: ");
  Serial.print(brightness);
  Serial.print(" (");
  Serial.print((brightness * 100) / 255);
  Serial.println("%)");
  Serial.print("Mode: ");
  switch(mode) {
    case 0: Serial.println("Steady"); break;
    case 1: Serial.println("Slow Blink"); break;
    case 2: Serial.println("Fast Blink"); break;
    case 3: Serial.println("SOS"); break;
    default: Serial.println("Unknown"); break;
  }
  Serial.print("Packet Sequence: ");
  Serial.println(packetSequence);
  Serial.println("===========================\n");
}

void processSerialCommand() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if (cmd == "ON") {
      ledState = true;
      Serial.println("[CMD] LED ON");
      sendCurrentState(false);
    }
    else if (cmd == "OFF") {
      ledState = false;
      Serial.println("[CMD] LED OFF");
      sendCurrentState(false);
    }
    else if (cmd.startsWith("BRIGHTNESS ")) {
      int val = cmd.substring(11).toInt();
      if (val >= 0 && val <= 255) {
        brightness = val;
        Serial.print("[CMD] Brightness set to ");
        Serial.println(brightness);
        sendCurrentState(false);
      } else {
        Serial.println("[ERROR] Brightness must be 0-255");
      }
    }
    else if (cmd.startsWith("MODE ")) {
      int val = cmd.substring(5).toInt();
      if (val >= 0 && val <= 3) {
        mode = val;
        Serial.print("[CMD] Mode set to ");
        Serial.println(mode);
        sendCurrentState(false);
      } else {
        Serial.println("[ERROR] Mode must be 0-3 (0=steady, 1=slow, 2=fast, 3=SOS)");
      }
    }
    else if (cmd == "STATUS") {
      printStatus();
    }
    else if (cmd == "HELP") {
      Serial.println("\nAvailable Commands:");
      Serial.println("  ON                  - Turn LED on");
      Serial.println("  OFF                 - Turn LED off");
      Serial.println("  BRIGHTNESS <0-255>  - Set brightness");
      Serial.println("  MODE <0-3>          - Set mode (0=steady, 1=slow, 2=fast, 3=SOS)");
      Serial.println("  STATUS              - Show current status");
      Serial.println("  HELP                - Show this help\n");
    }
    else {
      Serial.println("[ERROR] Unknown command. Type HELP for commands.");
    }
  }
}

void updateControllerLED() {
  if (!ledState) {
    // LED is off
    analogWrite(LED_PIN, 0);
    return;
  }

  unsigned long currentTime = millis();

  switch (mode) {
    case 0:  // Steady
      analogWrite(LED_PIN, brightness);
      break;

    case 1:  // Slow blink (1 Hz - 500ms on, 500ms off)
      if (currentTime - lastControllerBlinkTime >= 500) {
        lastControllerBlinkTime = currentTime;
        controllerBlinkState = !controllerBlinkState;
      }
      analogWrite(LED_PIN, controllerBlinkState ? brightness : 0);
      break;

    case 2:  // Fast blink (4 Hz - 125ms on, 125ms off)
      if (currentTime - lastControllerBlinkTime >= 125) {
        lastControllerBlinkTime = currentTime;
        controllerBlinkState = !controllerBlinkState;
      }
      analogWrite(LED_PIN, controllerBlinkState ? brightness : 0);
      break;

    case 3:  // SOS pattern (... --- ...)
      {
        // SOS: 3 short (200ms), 3 long (600ms), 3 short (200ms), pause (1000ms)
        const unsigned int sosTiming[] = {200, 200, 200, 600, 600, 600, 200, 200, 200, 1000};
        const int sosCount = 10;

        if (currentTime - lastControllerBlinkTime >= sosTiming[controllerSosIndex]) {
          lastControllerBlinkTime = currentTime;
          controllerSosIndex = (controllerSosIndex + 1) % sosCount;
          // Odd indices are gaps, even are pulses
          controllerBlinkState = (controllerSosIndex % 2 == 0);
        }
        analogWrite(LED_PIN, controllerBlinkState ? brightness : 0);
      }
      break;

    default:
      analogWrite(LED_PIN, brightness);
      break;
  }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=================================");
  Serial.println("Stage 1 Controller - ESP-NOW POC");
  Serial.println("=================================\n");

  // Configure button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("[OK] Button configured on GPIO 15");

  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[OK] LED configured on GPIO 16");

  // Initialize WiFi in Station mode
  WiFi.mode(WIFI_STA);
  Serial.print("[OK] WiFi MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW initialization failed!");
    return;
  }
  Serial.println("[OK] ESP-NOW initialized");

  // Register callbacks
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceived);
  Serial.println("[OK] ESP-NOW callbacks registered");

  // Add specific scoreboard peers
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;  // Use current channel
  peerInfo.encrypt = false;

  // Add Scoreboard 1
  memcpy(peerInfo.peer_addr, SCOREBOARD_1_MAC, 6);
  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.print("[OK] Scoreboard 1 peer added: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", SCOREBOARD_1_MAC[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  } else {
    Serial.println("[ERROR] Failed to add Scoreboard 1 peer");
  }

  // Add Scoreboard 2
  memcpy(peerInfo.peer_addr, SCOREBOARD_2_MAC, 6);
  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.print("[OK] Scoreboard 2 peer added: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", SCOREBOARD_2_MAC[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  } else {
    Serial.println("[ERROR] Failed to add Scoreboard 2 peer");
  }

  // Flash LED 3 times to indicate ready
  Serial.println("\n[READY] Press button to toggle LEDs");
  Serial.println("===============================================\n");

  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Process serial commands
  processSerialCommand();

  // Read button state (active LOW - pressed when LOW)
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // Debounced button press detection with short/long press support
  static bool wasPressed = false;
  static unsigned long pressStartTime = 0;

  if (buttonPressed && !wasPressed) {
    // Button just pressed - record time
    pressStartTime = millis();
    wasPressed = true;
  }
  else if (!buttonPressed && wasPressed) {
    // Button just released - check duration
    unsigned long pressDuration = millis() - pressStartTime;
    unsigned long currentTime = millis();

    // Debounce check
    if (currentTime - lastButtonPress > DEBOUNCE_MS) {
      lastButtonPress = currentTime;

      if (pressDuration >= LONG_PRESS_MS) {
        // Long press - cycle brightness
        brightnessIndex = (brightnessIndex + 1) % 4;
        brightness = BRIGHTNESS_LEVELS[brightnessIndex];

        Serial.println("\n--- Long Press ---");
        Serial.print("Brightness: ");
        Serial.print(brightness);
        Serial.print(" (");
        Serial.print((brightness * 100) / 255);
        Serial.println("%)");

        sendCurrentState(false);
      }
      else {
        // Short press - toggle LED
        ledState = !ledState;

        Serial.println("\n--- Button Press ---");
        Serial.print("New LED State: ");
        Serial.println(ledState ? "ON" : "OFF");

        sendCurrentState(false);
      }
    }

    wasPressed = false;
  }

  // Update controller LED based on current state, brightness, and mode
  updateControllerLED();

  // Periodic heartbeat - send current state every 3 seconds
  unsigned long currentTime = millis();
  if (currentTime - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeat = currentTime;
    Serial.println("\n[HEARTBEAT] Sending periodic state update");
    sendCurrentState(false);
  }

  // Small delay to prevent excessive CPU usage
  delay(10);
}
