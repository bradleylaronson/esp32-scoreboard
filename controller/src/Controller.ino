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
uint32_t packetSequence = 0;     // Packet counter
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_MS = 200;  // 200ms debounce

// Heartbeat for periodic state broadcast
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL_MS = 3000;  // 3 seconds

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void sendCurrentState(bool isResponse = false);

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
  // Read button state (active LOW - pressed when LOW)
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // Debounced button press detection
  if (buttonPressed) {
    unsigned long currentTime = millis();

    // Check if enough time has passed since last press (debounce)
    if (currentTime - lastButtonPress > DEBOUNCE_MS) {
      lastButtonPress = currentTime;

      // Toggle LED state
      ledState = !ledState;

      Serial.println("\n--- Button Pressed ---");
      Serial.print("New LED State: ");
      Serial.println(ledState ? "ON" : "OFF");

      // Update controller LED
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);

      // Send current state to scoreboards
      sendCurrentState(false);

      // Wait for button release (simple approach)
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }

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
