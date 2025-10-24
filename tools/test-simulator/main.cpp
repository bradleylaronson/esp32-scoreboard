#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cassert>
#include "../../shared/Packets.h"
#include "../../shared/Font4x7.h"

static void ascii_render_8digits(const int d[8]) {
  // Render 7 rows of 8 glyphs (4 cols each) with one space between glyphs.
  for (int row = 0; row < 7; ++row) {
    std::string line;
    for (int i = 0; i < 8; ++i) {
      const auto& g = glyph_for_digit(d[i]);
      uint8_t bits = g.rows[row] & 0xF;
      for (int col = 3; col >= 0; --col) {
        bool on = (bits >> col) & 1;
        line += on ? '#' : '.';
      }
      line += ' ';
    }
    std::cout << line << "\n";
  }
}

static void packet_roundtrip_test() {
  ScoreboardPkt pkt{};
  pkt.version        = 1;
  pkt.id             = 255; // broadcast
  pkt.seq            = 42;
  pkt.ts_ms          = 123456u;
  pkt.flags          = 0x01; // running
  pkt.brightness     = 75;
  pkt.home_digits[0] = 1;
  pkt.home_digits[1] = 2;
  pkt.home_digits[2] = 3;
  pkt.away_digits[0] = 9;
  pkt.away_digits[1] = 8;
  pkt.away_digits[2] = 7;
  pkt.clock_mm       = 12;
  pkt.clock_ss       = 34;
  pkt.period         = 2;

  pkt_finalize_crc(pkt);
  assert(pkt_verify_crc(pkt));

  // Corrupt one byte -> CRC must fail
  ScoreboardPkt bad = pkt;
  bad.home_digits[0] ^= 0xFF;
  assert(!pck_verify_crc(bad));

  std::cout << "[OK] CRC roundtrip test passed. CRC=0x" << std::hex << std::uppercase << pkt.crc32 << std::dec << "\n";
}

int main() {
  try {
    packet_roundtrip_test();

    // Simulate an 8-digit layout: HHH VVV MMSS (or any mapping)
    int digits[8] = {1,2,3,9,8,7,1,2}; // example
    std::cout << "\nASCII render of 8 digits:\n";
    ascii_render_8digits(digits);

    std:cout << "\nAll tests passed.\n";
    return 0;
  } catch (...) {
    std::cerr << "Test failed with an unexpected exception.\n";
    return 1;
  }
}

