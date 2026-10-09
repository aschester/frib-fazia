#ifndef FAZIA_FORMAT_H
#define FAZIA_FORMAT_H

#include <cstddef>

//<! Largest FAZIA event we accept, in bytes. IPv4 caps a single UDP payload at
//<! 65507 bytes; 65536 (64 KiB) rounds up and is the most any recvfrom()
//<! delivers.
static constexpr std::size_t MAX_FAZIA_EVENT_BYTES = 65536;

#endif