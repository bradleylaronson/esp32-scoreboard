#pragma once
#include <cstdint>
#include <cstring>

// ============================================================================
// STAGE 1 PACKETS - Simple proof-of-concept packets
// ============================================================================

// Scoreboard to Controller: Request current state on boot
#pragma pack(push, 1)
struct StateRequest {
  uint8_t scoreboardID;  // 0 or 1 to identify which scoreboard is requesting
  uint8_t magic;         // 0xAA - helps identify packet type
};
#pragma pack(pop)

// Controller to Scoreboard: Current state (used for button press, heartbeat, and responses)
#pragma pack(push, 1)
struct Stage1Packet {
  uint8_t ledState;     // 0 = OFF, 1 = ON
  uint32_t sequence;    // Packet sequence number
};
#pragma pack(pop)

// ============================================================================
// PRODUCTION PACKET - Full scoreboard packet with CRC
// ============================================================================

// Wire format (keep packed for consistent CRC)
#pragma pack(push, 1)
struct ScoreboardPkt {
  uint8_t  version;        // 1
  uint8_t  id;             // 0/1 or 255=broadcast
  uint16_t seq;            // monotonic
  uint32_t ts_ms;          // controller ms timestamp
  uint8_t  flags;          // bit0=clockRunning, bit1=hardReset
  uint8_t  brightness;     // 0-100
  uint8_t  home_digits[3]; // e.g., [hundreds, tens, ones] or [blank, tens, ones]
  uint8_t  away_digits[3];
  uint8_t  clock_mm;       // 0-99
  uint8_t  clock_ss;       // 0-59
  uint8_t  period;         // 0(off),1..4
  uint8_t  reserved[6];    // future
  uint32_t crc32;          // must be last
};
#pragma pack(pop)

// basic CRC32 (poly 0xEDB88320), small + portable.
inline uint32_t crc32_calc(const void* data, size_t len) {
  const uint8_t* p = static_cast<const uint8_t*>(data);
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= p[i];
    for (int k = 0; k < 8; ++k){
      const uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

inline void pkt_finalize_crc(ScoreboardPkt& pkt) {
  pkt.crc32 = 0u;
  pkt.crc32 = crc32_calc(&pkt, sizeof(ScoreboardPkt));
}

inline bool pkt_verify_crc(const ScoreboardPkt& pkt) {
  return crc32_calc(&pkt, sizeof(ScoreboardPkt)) == pkt.crc32;
}

