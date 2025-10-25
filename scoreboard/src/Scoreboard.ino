/*
 * Stage 1 Scoreboard - ESP-NOW Proof of Concept
 *
 * Hardware:
 * - LED on GPIO 2 (through 220Ω resistor to GND)
 * - LED state controlled by packets from controller
 *
 * Setup Instructions:
 * 1. Upload this firmware to both scoreboard ESP32s
 * 2. Open serial monitor to see MAC address
 * 3. Note the MAC address and add it to the controller firmware
 * 4. Power on controller and press button to test
 */

#include <WiFi.h>
#include <esp_now.h>
#include "../../shared/Packets.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// LED GPIO
#define LED_PIN 16  // Changed from 2 - safer pin choice

// Scoreboard ID (optional - can be set differently for each board)
// Set to 0 or 1 to identify which scoreboard this is
#define SCOREBOARD_ID 0  // Change to 1 for the second scoreboard

// Controller MAC Address (from Dev_Addresses.txt)
uint8_t CONTROLLER_MAC[] = {0x44, 0x1D, 0x64, 0xF8, 0xFD, 0x84};

// ============================================================================
// GLOBAL STATE
// ============================================================================

uint32_t lastSequence = 0;
uint32_t packetsReceived = 0;
uint32_t packetsDropped = 0;
bool ledState = false;

// State sync management
bool stateSynced = false;
unsigned long bootTime = 0;
const unsigned long SYNC_TIMEOUT_MS = 5000;  // Wait max 5 seconds for sync

// ============================================================================
// ESP-NOW CALLBACKS
// ============================================================================

void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len) {
  // Check if packet is the right size
  if (len != sizeof(Stage1Packet)) {
    Serial.print("[WARNING] Received packet with wrong size: ");
    Serial.print(len);
    Serial.print(" bytes (expected ");
    Serial.print(sizeof(Stage1Packet));
    Serial.println(" bytes)");
    return;
  }

  // Cast data to our packet structure
  Stage1Packet *packet = (Stage1Packet*)data;

  // Detect dropped packets (sequence number gap)
  if (packetsReceived > 0) {
    uint32_t expectedSeq = lastSequence + 1;
    if (packet->sequence != expectedSeq) {
      uint32_t dropped = packet->sequence - expectedSeq;
      packetsDropped += dropped;
      Serial.print("[WARNING] Dropped ");
      Serial.print(dropped);
      Serial.println(" packet(s)");
    }
  }

  lastSequence = packet->sequence;
  packetsReceived++;

  // Print packet info
  Serial.println("\n--- Packet Received ---");
  Serial.print("From MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  Serial.print("Sequence: ");
  Serial.println(packet->sequence);
  Serial.print("LED State: ");
  Serial.println(packet->ledState ? "ON" : "OFF");

  // Update LED state
  ledState = (packet->ledState == 1);
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  Serial.print("LED set to: ");
  Serial.println(ledState ? "ON" : "OFF");

  // Mark as synced if this is the first packet
  if (!stateSynced) {
    stateSynced = true;
    Serial.println("[SYNCED] State synchronized with controller!");
  }

  // Print statistics
  Serial.print("Total packets: ");
  Serial.print(packetsReceived);
  Serial.print(" | Dropped: ");
  Serial.println(packetsDropped);
  Serial.println("-----------------------\n");
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n====================================");
  Serial.print("Stage 1 Scoreboard #");
  Serial.print(SCOREBOARD_ID);
  Serial.println(" - ESP-NOW POC");
  Serial.println("====================================\n");

  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[OK] LED configured on GPIO 16");

  // Initialize WiFi in Station mode
  WiFi.mode(WIFI_STA);

  // Print MAC address (important - needed for controller setup)
  Serial.println("\n*** IMPORTANT: Copy this MAC address ***");
  Serial.print("*** MAC Address: ");
  Serial.print(WiFi.macAddress());
  Serial.println(" ***");
  Serial.println("*** Add this to Controller.ino SCOREBOARD_x_MAC ***\n");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW initialization failed!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("[OK] ESP-NOW initialized");

  // Register receive callback
  esp_now_register_recv_cb(onDataReceived);
  Serial.println("[OK] Receive callback registered");

  // Add controller as peer so we can send state requests
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, CONTROLLER_MAC, 6);

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.print("[OK] Controller peer added: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", CONTROLLER_MAC[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  } else {
    Serial.println("[ERROR] Failed to add controller peer");
  }

  // Send state request to controller
  StateRequest request;
  request.scoreboardID = SCOREBOARD_ID;
  request.magic = 0xAA;

  Serial.print("[BOOT] Sending state request to controller (ID: ");
  Serial.print(SCOREBOARD_ID);
  Serial.println(")");

  esp_err_t result = esp_now_send(CONTROLLER_MAC, (uint8_t*)&request, sizeof(request));
  if (result == ESP_OK) {
    Serial.println("[OK] State request sent");
  } else {
    Serial.println("[WARNING] Failed to send state request - will sync from heartbeat");
  }

  // Record boot time for sync timeout
  bootTime = millis();

  // Flash LED 3 times to indicate ready
  Serial.println("\n[READY] Waiting for state sync from controller...");
  Serial.println("==============================================\n");

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
  // Check for sync timeout (only once)
  static bool timeoutWarningShown = false;
  if (!stateSynced && !timeoutWarningShown) {
    unsigned long currentTime = millis();
    if (currentTime - bootTime > SYNC_TIMEOUT_MS) {
      Serial.println("\n[WARNING] State sync timeout - controller may be offline");
      Serial.println("[INFO] Will sync from next heartbeat or button press\n");
      timeoutWarningShown = true;
    }
  }

  // Nothing to do in loop - all packet handling is done in callback
  // Just keep the watchdog happy
  delay(100);

  // Optional: Print periodic status every 30 seconds
  static unsigned long lastStatusPrint = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastStatusPrint > 30000) {
    lastStatusPrint = currentTime;

    if (packetsReceived > 0) {
      Serial.println("\n========== Status Update ==========");
      Serial.print("Uptime: ");
      Serial.print(currentTime / 1000);
      Serial.println(" seconds");
      Serial.print("Packets received: ");
      Serial.println(packetsReceived);
      Serial.print("Packets dropped: ");
      Serial.println(packetsDropped);
      Serial.print("Current LED state: ");
      Serial.println(ledState ? "ON" : "OFF");
      Serial.println("===================================\n");
    }
  }
}
