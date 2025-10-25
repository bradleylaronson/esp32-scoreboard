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

// ============================================================================
// CONFIGURATION
// ============================================================================

// Button GPIO
#define BUTTON_PIN 2

// LED GPIO
#define LED_PIN 4

// Scoreboard MAC Addresses
// TODO: Update these with actual MAC addresses from your scoreboard ESP32s
// Format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
uint8_t SCOREBOARD_1_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // REPLACE ME
uint8_t SCOREBOARD_2_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // REPLACE ME

// Use broadcast address for initial testing (reaches all ESP-NOW devices)
uint8_t BROADCAST_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ============================================================================
// SIMPLE PACKET STRUCTURE FOR STAGE 1
// ============================================================================

struct Stage1Packet {
  uint8_t ledState;     // 0 = OFF, 1 = ON
  uint32_t sequence;    // Packet sequence number
} __attribute__((packed));

// ============================================================================
// GLOBAL STATE
// ============================================================================

bool ledState = false;           // Current LED state (off/on)
uint32_t packetSequence = 0;     // Packet counter
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_MS = 200;  // 200ms debounce

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
  Serial.println("[OK] Button configured on GPIO 2");

  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[OK] LED configured on GPIO 4");

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

  // Register send callback
  esp_now_register_send_cb(onDataSent);

  // Add broadcast peer for initial testing
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;  // Use current channel
  peerInfo.encrypt = false;

  // Add broadcast peer
  memcpy(peerInfo.peer_addr, BROADCAST_MAC, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ERROR] Failed to add broadcast peer");
  } else {
    Serial.println("[OK] Broadcast peer added");
  }

  // Optional: Add specific scoreboard peers (uncomment when MAC addresses are known)
  /*
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
  */

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

      // Create packet
      Stage1Packet packet;
      packet.ledState = ledState ? 1 : 0;
      packet.sequence = packetSequence++;

      // Send packet to broadcast address
      esp_err_t result = esp_now_send(BROADCAST_MAC,
                                       (uint8_t*)&packet,
                                       sizeof(packet));

      if (result == ESP_OK) {
        Serial.print("Packet #");
        Serial.print(packet.sequence);
        Serial.println(" queued for transmission");
      } else {
        Serial.println("ERROR: Failed to queue packet!");
      }

      // Wait for button release (simple approach)
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }

  // Small delay to prevent excessive CPU usage
  delay(10);
}
