# Changelog

## []

- Message tagging support. Input (and other intermediate) messages can be tagged and filtered before use or output. See `tags`, `itags` and `otags`.
- Better support for AGNSS via supl.google.com
- Added support for missing AGNSS data using `--ad-assisted-gnss`
- Support for single-shot request for assistance data
- Support to specify if the cell is GSM using `--gsm-cell`
- Added `--li-disable` to disable location information
- Added `--ls-hack-never-send-abort` to bypass incorrect abort handling in location server
- Initial cell will be used on reconnect. Use `--use-latest-cell-on-reconnect` to re-enable the previous behaviour
- Initial cell will not be overwritten with the one from the control interface until the first RequestAssistanceData has been sent
- Fix bug where inital client message still had color even when `--log-no-color` was enabled
- Change color used by `INFO` logs from `WHITE` to `FOREGROUND` 

## [4.0.21] 2025-07-16

- Implement update_assistance_data

## [4.0.19] 2025-07-16

- Set `TCP_USER_TIMEOUT`

## [4.0.18] 2025-07-15

- Shutdown socket on disconnect
- Handle SUPL session termination in LPP

## [4.0.17] 2025-07-14

- Disconnect on failed SUPL send
- Fake location is now pushed through streamline
- Support NMEA location without GST/EPE and VTG
- Update posSIB logging names to conform to 3GPP LPP standard
- `--li-unsolicited` implemented
- `--ad-disable` implemented
- Fix CMake truthiness checking on variables

## [4.0.16] 2025-06-04

- Fix incorrect handling of GST message with missing fields
- Add support to output location information as json
- Fix grid point bitmasking

## [4.0.15] 2025-05-20

- Fixed bug when using `llh_to_ecef` to generate the virtual reference station for Tokoro
- Include per satelite/signal information in posSIB log
- Added system to log data for posSIBs
- Included `LPP-Broadcast-Definitions.asn` and generated the corresponding ASN.1 C source files

## [4.0.14] 2025-04-14

- Added `--ls-hack-bad-transaction-initiator` to bypass incorrect location server behaviour
- Added logging when sending or receive LPP message

## [4.0.11] 2025-04-07

- Include `assistanceDataSupportList` in `ProvideCapabilities` for the default message send by the LPP Client
- Only include GNSS supported by assistance data in `gnss-SupportList`

## [4.0.10] 2025-04-03

- More information included when printing SUPL message with `supl/print`
- Added logging support of LPP messages via `lpp/print`

## [4.0.9] 2025-04-03 

- Fix `agpsSETassisted`, `agpsSETBased`, and `agpsSETBasedPreferred`

## [4.0.8] 2025-03-27 

- Updated loglet to use a new system for modules
- Added `--log-tree` to visualize the logging modules and their logging level
- Added `--ls-interface` to set the interface used by the location server connection

## [4.0.7] 2025-03-20

- Fix decoding of IMSI identity

## [4.0.6] 2025-03-18

- Include Application ID details in SUPL
- Use the initial cell in SUPL instead of a dummy cell
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
