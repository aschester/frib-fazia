# FRIB-FAZIA integration

## Requirements
* Debian 12 (bookworm)
* FRIBDAQ 12.2

## Contents
* faziaReadout - SBS readout for FAZIA to read data directly via UDP. Untested, but its a starting point.
* faziaToRingBuffer - Reads FAZIA data on stdin, writes to a FRIBDAQ ringbuffer sink.
* faziaToRingItem - Reads FAZIA data on stdin, writes FRIBDAQ ring items on stdout.