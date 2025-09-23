# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- C++11 compatibility support via `CMAKE_CXX_STANDARD` variable
- WIP positioning engine `idokeido`
- Input/output stage with `chain=tlf` support
- Stage `tlf` for reading and writing time-logged format data
- Function performance logging with `LOG_FUNCTION_PERFORMANCE=ON`
- Chunked-log output interface for hourly rotating log files with timestamp names

### Fixed
- SerialInput incorrectly treating set_fd return value as error
- Output assert when chain/stages not specified
- Forward task and missing data during fast reads

## [4.0.22] - 2025-08-07

### Added
- Satellite parameters in data-tracing
- Message tagging support with `tags`, `itags` and `otags` options
- Support for missing AGNSS data using `--ad-assisted-gnss`
- Single-shot request support for assistance data
- GSM cell specification using `--gsm-cell`
- `--li-disable` option to disable location information
- `--ls-hack-never-send-abort` to bypass incorrect abort handling in location server
- `--use-latest-cell-on-reconnect` option for reconnection behavior

### Changed
- Initial cell used on reconnect by default
- Initial cell preservation until first RequestAssistanceData is sent
- INFO log color changed from WHITE to FOREGROUND
- Improved AGNSS support via supl.google.com

### Fixed
- Initial client message color when `--log-no-color` enabled 

## [4.0.21] - 2025-07-16

### Added
- Implementation of update_assistance_data

## [4.0.19] - 2025-07-16

### Changed
- Set TCP_USER_TIMEOUT for socket connections

## [4.0.18] - 2025-07-15

### Added
- Socket shutdown on disconnect
- SUPL session termination handling in LPP

## [4.0.17] - 2025-07-14

### Added
- `--li-unsolicited` option
- `--ad-disable` option
- NMEA location support without GST/EPE and VTG

### Changed
- Disconnect on failed SUPL send
- Fake location now pushed through streamline
- Updated posSIB logging names to conform to 3GPP LPP standard

### Fixed
- CMake truthiness checking on variables

## [4.0.16] - 2025-06-04

### Added
- JSON output support for location information

### Fixed
- Incorrect handling of GST message with missing fields
- Grid point bitmasking

## [4.0.15] - 2025-05-20

### Added
- Per satellite/signal information in posSIB log
- System to log data for posSIBs
- LPP-Broadcast-Definitions.asn and corresponding ASN.1 C source files

### Fixed
- Bug when using llh_to_ecef to generate virtual reference station for Tokoro

## [4.0.14] - 2025-04-14

### Added
- `--ls-hack-bad-transaction-initiator` to bypass incorrect location server behavior
- Logging when sending or receiving LPP messages

## [4.0.11] - 2025-04-07

### Changed
- Include assistanceDataSupportList in ProvideCapabilities for default LPP Client message
- Only include GNSS supported by assistance data in gnss-SupportList

## [4.0.10] - 2025-04-03

### Added
- Enhanced SUPL message information when printing with supl/print
- LPP message logging support via lpp/print

## [4.0.9] - 2025-04-03

### Fixed
- agpsSETassisted, agpsSETBased, and agpsSETBasedPreferred functionality

## [4.0.8] - 2025-03-27

### Added
- `--log-tree` to visualize logging modules and their levels
- `--ls-interface` to set location server connection interface

### Changed
- Updated loglet to use new module system

## [4.0.7] - 2025-03-20

### Fixed
- IMSI identity decoding

## [4.0.6] - 2025-03-18

### Added
- Application ID details in SUPL
- `--ad-delivery-amount` option
- `--ad-no-antenna-height` option

### Changed
- Use initial cell in SUPL instead of dummy cell

## [4.0.5] - 2025-03-13

### Changed
- `--log-no-color` no longer prints reset color code
- Updated digit order of SUPL identity encoding/decoding

## [4.0.4] - 2025-03-13

### Added
- `--log-no-color` to disable colored logging output
- `--log-flush` to flush logging after every line

## [4.0.3] - 2025-03-12

### Added
- Container building and publishing to GHCR

## [4.0.2] - 2025-03-12

### Added
- TRACE/VERBOSE/DEBUG availability in Release mode
- `--output-print-everything` option to override print=true for all outputs
- Experimental `--output tcp-server:` for TCP server output to connected clients
- Test output format sending TESTTESTTESTTEST every second for output validation

## [4.0.1] - 2025-03-07

### Changed
- Major architectural overhaul from version 3.x
- Comprehensive changelog documentation starting from version 4.0

**Note:** Due to extensive changes between versions 3.x and 4.0, detailed migration information is available in the [Upgrade Guide](/UPGRADE_FROM_V3.md).
