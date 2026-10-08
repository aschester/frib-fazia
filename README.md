# FRIB-FAZIA integration

## Requirements
* Debian 12 (bookworm)
* FRIBDAQ 12.2
* Assumes host byte order is little endian (x86_64 architecture)

## Contents
* faziaReadout - SBS readout for FAZIA to read data directly via UDP. Untested, but its a starting point.
* faziaToFrib - Reads FAZIA data on stdin, writes FRIBDAQ ring items to a configurable sink (ring buffer, file, or stdout) selected with `--sink`.