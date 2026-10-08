# FRIB-FAZIA integration

## Requirements
* Debian 12 (bookworm)
* FRIBDAQ 12.2

## Contents
* faziaReadout - SBS readout for FAZIA to read data directly via UDP. Untested, but its a starting point.
* faziaToFrib - Reads FAZIA data on stdin, writes FRIBDAQ ring items to a configurable sink (ring buffer, file, or stdout) selected with `--sink`.