#!/usr/bin/env python3
"""
fazia_test_source.py - sends synthetic FAZIA-framed UDP test data.

Wire format per packet (one packet = one fragment/event):
    [8 bytes: uint64_t timestamp, little-endian]
    [8 bytes: uint64_t evtSize,   little-endian - payload size in bytes]
    [evtSize bytes: payload]

Payload is a run of 16-bit words 0x1111, 0x2222, 0x3333, ... (wrapping mod
0x10000), little-endian, for easy visual identification in a hex dump.
"""

import argparse
import socket
import struct
import sys
import time


def build_payload(nwords: int) -> bytes:
    """
    Build a payload of nwords 16-bit words, each word = 0x1111 * (i + 1) mod 0x10000,
    where i = 0..nwords-1. Returns the payload as a bytes object, little-endian.
    """
    words = [(0x1111 * (i + 1)) & 0xFFFF for i in range(nwords)]
    return struct.pack(f"<{nwords}H", *words)


def build_packet(timestamp: int, payload: bytes) -> bytes:
    """
    Build a packet with the given timestamp and payload. The packet consists of:
    - 8 bytes: uint64_t timestamp (little-endian)
    - 8 bytes: uint64_t evtSize (little-endian, size of payload in bytes)
    - payload bytes
    """
    return struct.pack("<QQ", timestamp, len(payload)) + payload


def main():
    """
    Main function to parse command-line arguments and send synthetic FAZIA-framed UDP test data.
    """
    p = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    p.add_argument("--host", required=True, help="destination host/IP")
    p.add_argument("--port", required=True, type=int, help="destination UDP port")
    p.add_argument(
        "--count",
        type=int,
        default=0,
        help="number of fragments to send (default: 0 = run until Ctrl+C)",
    )
    p.add_argument(
        "--rate",
        type=float,
        default=10.0,
        help="fragments per second (default: 10; use 0 for no pacing)",
    )
    p.add_argument(
        "--words",
        type=int,
        default=8,
        help="16-bit words in each fragment's payload (default: 8)",
    )
    p.add_argument(
        "--ts-start",
        type=int,
        default=1000,
        help="starting synthetic timestamp value (default: 1000)",
    )
    p.add_argument(
        "--ts-step",
        type=int,
        default=1000,
        help="amount the timestamp increases per fragment (default: 1000)",
    )
    args = p.parse_args()

    payload = build_payload(args.words)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = (args.host, args.port)

    timestamp = args.ts_start
    sent = 0
    delay = (1.0 / args.rate) if args.rate > 0 else 0.0

    print(
        f"Sending to {args.host}:{args.port} "
        f"({args.words} words/fragment, ts step {args.ts_step}, "
        f"rate {'unlimited' if delay == 0 else f'{args.rate}/s'})"
    )

    try:
        while args.count == 0 or sent < args.count:
            sock.sendto(build_packet(timestamp, payload), dest)
            sent += 1
            timestamp += args.ts_step
            if delay:
                time.sleep(delay)
    except KeyboardInterrupt:
        pass
    finally:
        sock.close()
        print(f"\nSent {sent} fragments; last timestamp = {timestamp - args.ts_step}")


if __name__ == "__main__":
    main()
