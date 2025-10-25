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

// ============================================================================
// CONFIGURATION
// ============================================================================

// LED GPIO
#define LED_PIN 2

// Scoreboard ID (optional - can be set differently for each board)
// Set to 0 or 1 to identify which scoreboard this is
#define SCOREBOARD_ID 0  // Change to 1 for the second scoreboard

// ============================================================================
// PACKET STRUCTURE (must match controller)
// ============================================================================

struct Stage1Packet {
  uint8_t ledState;     // 0 = OFF, 1 = ON
  uint32_t sequence;    // Packet sequence number
} __attribute__((packed));

// ============================================================================
// GLOBAL STATE
// ============================================================================

uint32_t lastSequence = 0;
uint32_t packetsReceived = 0;
uint32_t packetsDropped = 0;
bool ledState = false;

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
  Serial.println("[OK] LED configured on GPIO 2");

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

  // Flash LED 3 times to indicate ready
  Serial.println("\n[READY] Waiting for packets from controller...");
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
