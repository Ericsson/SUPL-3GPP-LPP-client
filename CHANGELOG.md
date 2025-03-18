# Changelog

## [] 

- Added `--ad-delivery-amount`
- Added `--ad-no-antenna-height`

## [4.0.5] 2025-03-13

- `--log-no-color` will also not print the reset color code
- Update the digit order of SUPL identity encoding/decoding  

## [4.0.4] 2025-03-13

- Added `--log-no-color` to disable colored logging output
- Added `--log-flush` to flush logging after every line

## [4.0.3] 2025-03-12

- Building and publishing containers to GHCR

## [4.0.2] 2025-03-12

- TRACE/VERBOSE/DEBUG is now available in Release mode
- Added `--output-print-everything` option to set/override `print=true` for every output
- Added experimental `--output tcp-server:` for outputing as a TCP server to all connected clients.
- Added new output format `test`. The test format will send the string `TESTTESTTESTTEST` every second to all outputs that includes the `test` format. This is useful for testing if a output is working as expected. 

## [4.0.1] 2025-03-07

Due to extensive architectural changes and improvements between versions 3.x and 4.0, a comprehensive changelog was not maintained. The upgrade includes significant modifications to core functionality and structure. Starting from version 4.0, all changes will be thoroughly documented in this changelog.
