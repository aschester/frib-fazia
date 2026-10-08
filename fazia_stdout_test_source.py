#!/usr/bin/env python3
"""
fazia_test_source.py - writes FAZIA-framed test data to stdout.

Framed stream format (one record = one fragment/event):
    [ 8 bytes       : uint64_t timestamp, little-endian ]
    [ 8 bytes       : uint64_t evtSize,   little-endian ]  (payload size in bytes)
    [ evtSize bytes : payload                           ]

Payload is a run of 16-bit words 0x1111, 0x2222, 0x3333, ... (wrapping mod
0x10000), little-endian, for easy visual identification in a hex dump.

Pipe it into faziaToFrib, e.g.:
    ./fazia_test_source.py --count 5 | ./faziaToFrib --sink=-

NOTE: this is the on-stdout framing consumed by faziaToFrib. The real FAZIA
UDP wire format differs and is NOT produced here.
"""

import argparse
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
    Build a record with the given timestamp and payload. The record consists of:
    - 8 bytes: uint64_t timestamp (little-endian)
    - 8 bytes: uint64_t evtSize (little-endian, size of payload in bytes)
    - payload bytes
    """
    return struct.pack("<QQ", timestamp, len(payload)) + payload


def main():
    """
    Parse command-line arguments and write synthetic FAZIA-framed test data to stdout.
    """
    p = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    p.add_argument(
        "--count",
        type=int,
        default=0,
        help="number of fragments to write (default: 0 = run until Ctrl+C)",
    )
    p.add_argument(
        "--rate",
        type=float,
        default=0.0,
        help="fragments per second (default: 0 = no pacing, write as fast as possible)",
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
    out = sys.stdout.buffer  # binary stdout - the framed stream goes here

    timestamp = args.ts_start
    sent = 0
    delay = (1.0 / args.rate) if args.rate > 0 else 0.0

    # Status/diagnostics go to stderr so they never corrupt the binary stdout stream.
    print(
        f"Writing framed data to stdout "
        f"({args.words} words/fragment, ts step {args.ts_step}, "
        f"rate {'unlimited' if delay == 0 else f'{args.rate}/s'})",
        file=sys.stderr,
    )

    try:
        while args.count == 0 or sent < args.count:
            out.write(build_packet(timestamp, payload))
            out.flush()
            sent += 1
            timestamp += args.ts_step
            if delay:
                time.sleep(delay)
    except KeyboardInterrupt:
        pass
    except BrokenPipeError:
        # Downstream consumer closed the pipe (e.g. `| head`); exit quietly.
        sys.exit(0)
    finally:
        print(
            f"Wrote {sent} fragments; last timestamp = {timestamp - args.ts_step}",
            file=sys.stderr,
        )


if __name__ == "__main__":
    main()
