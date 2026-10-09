#ifndef FAZIA_FORMAT_H
#define FAZIA_FORMAT_H

#include <cstddef>
#include <cstdint>

/** @namespace fazia */
namespace fazia {

//<! Largest FAZIA event we accept, in bytes. IPv4 caps a single UDP payload at
//<! 65507 bytes; 65536 (64 KiB) rounds up and is the most any recvfrom()
//<! delivers.
static constexpr std::size_t MAX_EVENT_BYTES = 65536;

/** @namespace fazia::format */
namespace format {

// FAZIA 16-bit word grammar. A word is data when its MSB is 0, metadata when 1,
// with "tags" as the 0x7xxx (MSB-0) exception. Each type matches as
// (w & MASK) == VALUE; mask widths differ because types use 1/4/5/8 ident.
// bits.

inline constexpr std::size_t WORD = sizeof(uint16_t);

inline constexpr uint16_t MSB_MASK = 0x8000; // bit[15]: 0 = DATA
inline constexpr uint16_t DATA_VALUE = 0x0000;

inline constexpr uint16_t NIBBLE_MASK = 0xF000; // 4-bit types
inline constexpr uint16_t TAG_VALUE = 0x7000;   // 0x7xxx tag header
inline constexpr uint16_t LEN_VALUE = 0xA000;   // FEELEN / BLKLEN
inline constexpr uint16_t FEECRC_VALUE = 0xB000;
inline constexpr uint16_t BLKCRC_VALUE = 0xD000;
inline constexpr uint16_t EC_VALUE = 0xE000;     // Event counter
inline constexpr uint16_t FEEEOE_VALUE = 0xF000; // Trimmed by BC

inline constexpr uint16_t HDR5_MASK = 0xF800; // 5-bit types (share a nibble)
inline constexpr uint16_t DETHDR_VALUE = 0x9000;
inline constexpr uint16_t TELHDR_VALUE = 0x9800;
inline constexpr uint16_t BLKHDR_VALUE = 0xC000;

inline constexpr uint16_t EOE_MASK = 0xFF00;  // 8-bit type
inline constexpr uint16_t EOE_VALUE = 0xC800; // End of event

inline constexpr uint16_t PADDING = 0x8080; // Exact match

inline constexpr uint16_t LENGTH_MASK = 0x0FFF; // FEELEN / BLKLEN field
inline constexpr uint16_t BLKID_MASK = 0x07FF;  // BLKHDR field

// Tag identifiers. NOTE: the doc labels the FRIB-timestamp tag 0x7200 (collides
// with CENTRUM); real data carries it as 0x7300 (verified against a capture).
inline constexpr uint16_t TAG_FRIB_TS = 0x7300;

inline constexpr unsigned TS_BITS_PER_WORD = 15;
inline constexpr std::size_t TRIGGER_WORDS = 3; // fixed: TELHDR..DETHDR gap

} // namespace format
} // namespace fazia

#endif