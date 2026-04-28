# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

## [4.0.25] - 2026-04-28

### Added
- Location server TLS/mTLS support with pluggable backend (OpenSSL). New CLI flags: `--ls-tls`, `--ls-tls-skip-verify`, `--ls-ca-cert`, `--ls-client-cert`, `--ls-client-key`. Uses the system CA store by default; `--ls-ca-cert` overrides it. Supports separate or combined PEM for the client cert/key. The TLS handshake is fully integrated with the epoll scheduler (non-blocking, handles `SSL_ERROR_WANT_READ`/`WANT_WRITE` by re-subscribing to the relevant event). Requires `-DUSE_OPENSSL=ON`. See `examples/client/README.md` for details.
- SPARTN generator: support new Galileo signal slots L8Q/C8Q (E5 AltBOC, SF102/SF105 bit 3) and L6C/C6C (E6 C, bit 4) from SPARTN v2.0.3; support new BDS signal slots L7I/C7I (B2 I, SF103/SF106 bit 2) and L1P/C1P (B1C Pilot, bit 4). AltBOC corrections are now emitted natively on L8Q/C8Q instead of frequency-scaled to E5B Q.
- SPARTN generator: new three-stage bias pipeline (LPP→RINEX→RINEX-expanded→SPARTN slot) replaces the old `std::map`-based translation. Zero heap allocation in the hot path. New `--l2s-bias-map GNSS:FROM=TO[|FROM=TO]...` CLI flag adds runtime RINEX→RINEX signal relationships (e.g. `--l2s-bias-map 'BDS:5X=5P'` to derive B2a pilot biases from combined). Supports `L`/`C` prefix for phase/code-only mapping; short form applies to both.

### Fixed
- SPARTN generator: BDS_FREQ table for B2 I/Q/I+Q entries used B1 frequency (1561.098 MHz) instead of B2b (1207.14 MHz). No observable impact before but would have caused incorrect frequency-scaled corrections if B2 Q or B2 I+Q were ever mapped.

### Changed
- CLI validation and parse errors are now printed in red both above and below the help dump, so the reason is visible without scrolling.
- `examples/CMakeLists.txt`: each example now has its own `BUILD_EXAMPLE_*` option (all ON by default except `transform`)
- `docker/build-sizes.py`: expanded with per-option impact table, bloaty analysis, and new configs (`no-pie`, `no-xer-jer`, `absolute-min`); only builds `example-client`
- `dependency/lpp/message.cpp`, `session.cpp`: guard `xer_fprint`/`xer_encode` calls with `#ifndef ASN_DISABLE_XER_SUPPORT` to allow `ASN1_XER_SUPPORT=OFF` builds

## [4.0.24] - 2026-04-16

### Added
- SPARTN generator: expose CRC type, solution ID, and solution processor ID as CLI options

### Fixed
- SPARTN TF009 timestamps now use native GNSS system time per spec (BDS was 14s off, GLO was ~10782s off); rounding changed from round-to-nearest to floor
- SPARTN payload length encoding
- Tokoro: apply hydrostatic delta correction in mapping function (previously computed but not applied)
- Tag support in generators and related RTCM issue
- IO: support stdin and stdout simultaneously
- Test: regenerated RTCM golden files after RTCM_N2_31 scale fix (0x1p-32 → 0x1p-31)

### Changed
- Build: Docker images lowered glibc requirements for broader compatibility

## [4.0.23] - 2026-03-20

### Added
- Tag system: Global tag registry with descriptions and categories
- Tag system: --list-tags CLI flag with tree-like format similar to --log-tree
- Tag system: Tag aliases (e.g., "tok" for "tokoro", "ido" for "idokeido")
- Tag system: All ephemeris converters (lpp2eph, ubx2eph, rtcm2eph) now properly tag their output
- Tag system: tags namespace with convenience functions (tags::register_tag, tags::get, tags::to_string)
- Tag system: Tag and TagMask types with compile-time configurable size (default 64)
- Tag system: TagMask operators (|, &, |=, &=) and static filter function
- Tag system: Tag mask to string conversion for debug printing
- Time library: scalar operators (+/-/+=/-=) and operator- returning double for all time classes
- Time library: as_double() method for Timestamp (alias for full_seconds())
- Time library: comparison operators (< <= > >= == !=) for Tai, Utc, Gst, Bdt, Glo
- Time library: simplified Gps::from_week_tow(week, tow) overload combining integer and fractional TOW
- Time library: Tests demonstrating double precision loss vs Timestamp precision preservation

### Fixed
- Fixed 6 precision bugs in tokoro/data/gather.cpp using direct Tai comparison instead of full_seconds()
- Fixed SPARTN satellite mask size calculation using highest PRN instead of satellite count
- Fixed SPARTN BeiDou satellite mask sizes to match SF094 specification (37/46/55/64 bits)
- Fixed SPARTN BeiDou IODE (SF100) to use 8 bits per specification (was incorrectly 10 bits)
- Fixed SPARTN BeiDou time tags off by 14 seconds (BDS=TAI-33s vs GPS=TAI-19s)
- Fixed SPARTN GLONASS time tags off by ~3 hours (was ignoring UTC+3 Moscow offset and leap seconds)
- Fixed SPARTN bias mask size selection to use highest bias type instead of count

### Removed
- Removed SPARTN iode_shift parameter (was incorrectly defaulting to true)
- Removed tick_callbacks API from Scheduler (use defer() instead for deferred operations)
- Removed SocketTask class (use OwnedFileDescriptorTask instead)

### Changed
- Config::get_tag() now uses global tag registry instead of local storage
- Time library is now more ergonomic
- `TimeoutTask` now auto-schedules on construction (takes duration + callback)
- Scheduler now owns event registrations via ScheduledEvent handles with generation counters for safe event lifecycle management
- Task classes (TimeoutTask, PeriodicTask, FileDescriptorTask, ListenerTask, UdpListenerTask, TcpConnectTask) migrated to new abstracted EventInterest API
- Scheduler uses deferred slot deallocation (pending_free) to keep slot data valid throughout event processing frame
- Serial port is now configured with `VMIN=0` and `VTIME=0`
- CPM is now required to build
- Refactored coordinate frames to use FrameTrait pattern with modular file structure and temporal model classification
- Extracted SignalId and SatelliteId to new dependency/gnss library for better modularity
- Converted ephemeris test data from text to MessagePack format for faster test execution
- Fixed identifier naming
- Eliminated 1MB global buffer by using direct vfprintf formatting
- Hardened loglet to preserve errno, report internal errors, and avoid redundant fflush
- ForwardStreamTask no longer warns on EAGAIN/EWOULDBLOCK (expected when pipe is full)
- Streamline queue size default reduced from 2048 to 128 (configurable via EVENT_QUEUE_SIZE)
- SUPL internal library updated to use loglet properly with function tracing, correct asserts, and verbose output on early exits
- Example-client AGnssProcessor now takes cell and identity by const reference with validation
- Example-client A-GNSS GNSS flags now default to true and are masked by global GNSS settings
- Compiler test script now relies on CMake/Ninja incremental builds instead of manual caching
- Example-client A-GNSS now uses processor pattern with periodic and triggered modes
- CMake minimum version updated from 3.5 to 3.14 for CPM support
- Docker base images updated to install CMake 3.14+ from Kitware for Ubuntu 18.04
- Improved logging throughout modem dependency with detailed verbose output
- Enhanced RTCM parsing and bitset performance
- Better RTCM CRC check and logging
- Improved NMEA processor logging and information output
- Example-client lpp2eph now correctly uses SatelliteId to convert LPP satellite IDs to PRNs
- Example-client lpp2eph now correctly sets lpp_iod to full IOD bitstring value for SSR matching
- Tokoro tropospheric and ionospheric correction warnings now include specific failure reasons and satellite names

### Added
- Global scheduler with `current()`, `set_current()`, `has_current()` and `ScopedScheduler` RAII guard
- No-argument `schedule()` overloads on all task classes that use `scheduler::current()`
- `Timer` base class for timerfd management with arm/disarm/move support
- `RepeatableTimeoutTask` for reconnect/retry patterns with schedule/cancel/restart
- New Stream abstraction for bidirectional I/O with write buffering, read buffering, and epoll integration
- `--stream` CLI option for explicit stream declaration with `--input stream:id=...` and `--output stream:id=...` references
- Stream types: serial, tcp-client, tcp-server, udp-client, udp-server, fd, pty, stdio, file
- Serial `raw=true` option to skip termios configuration for virtual/pre-configured devices
- Coordinate representation library in dependency/coordinates with Eigen-based types (ECEF, LLH, ENU, NED, AER) and transformations using matrix operations
- Multi-reference frame coordinate transformations with template-based and graph-based APIs supporting WGS84, PZ-90, ITRF (88-2020), and CGCS2000
- ITRF transformation parameters from IERS Technical Note 36 for ITRF88 through ITRF2020
- Regression tests for coordinates library against tokoro and idokeido implementations
- Tokoro snapshot testing system, offline generation tool, and regression tests
- Phase alignment support for tokoro (enable with `--tkr-phase-alignment`)
- MessagePack serialization library with C++11 support
- Added GLONASS ephemeris support
- Added GLONASS time system support with UTC offset handling
- Added comprehensive GLONASS ephemeris tests with reference data
- Added Tokoro coordinate transformation tests (ECEF, ENU, WGS84)
- Added error handling framework with Error type, Result<T> wrapper
- Added ANTEX parser tests and libFuzzer-based fuzzing support
- Added QZSS ephemeris support
- Added QZSS satellite and signal ID mapping for RTCM
- Added comprehensive QZSS ephemeris tests with reference data
- Added MessagePack encoder/decoder library with support for all basic types, arrays, maps, and extensions
- Added BufferInput and BufferOutput classes for in-memory I/O operations
- Added ephemeris tests for GPS, Galileo, and BeiDou
- Added QZSS and NavIC support flags to SPARTN generator
- Added doctest testing framework with comprehensive time module tests (timestamp, GPS, UTC, TAI, BDT, GST, GLO conversions)
- Added NMEA, UBX, RTCM, and AT parser tests with libFuzzer-based fuzzing support
- Added verbose logging to UBX encoder/decoder for buffer overflow detection
- Added TypeName trait specializations for ephemeris types (GPS, Galileo, BeiDou)
- Added build version information including git commit, branch, build date, compiler, and platform
- Exposed new loglet options in example-client via `--log-no-report-errors` and `--log-no-stderr` flags
- Added loglet options: `set_report_errors()` and `set_use_stderr()`
- Optional static analyzers: clang-tidy, cppcheck, IWYU
- Input on_complete callback for detecting when inputs finish (EOF, disconnect)
- Auto-shutdown on input completion with `--input-shutdown-on-complete` and `--input-shutdown-delay`
- Scheduler interrupt() method to cleanly stop execute loops
- Scheduler defer() method to safely schedule cleanup callbacks after event processing
- Example-client A-GNSS processor now prevents concurrent requests
- Example-client lpp2eph processor to convert LPP assistance data to ephemeris messages
- Example-client ubx2eph processor to convert UBX messages to ephemeris messages
- Example-client rtcm2eph processor to convert RTCM messages to ephemeris messages
- Example-client tokoro now uses ephemeris inspector pattern instead of direct ephemeris processing
- SatelliteId prn() method to convert LPP ID to PRN
- Tokoro generator get_grid_cell_center_position() to get center of grid cell
- Example-client `--disable-lpp2eph` option to disable LPP to ephemeris conversion (enabled by default)
- Example-client `--disable-ubx2eph` option to disable UBX to ephemeris conversion (enabled by default)
- Example-client `--disable-rtcm2eph` option to disable RTCM to ephemeris conversion (enabled by default)
- Example-client `--l2e-no-gps`, `--l2e-no-gal`, `--l2e-no-bds` options for lpp2eph
- Example-client `--u2e-no-gps`, `--u2e-no-gal`, `--u2e-no-bds` options for ubx2eph
- Example-client `--r2e-no-gps`, `--r2e-no-gal`, `--r2e-no-bds` options for rtcm2eph
- LPP client now requests navigation model for A-GNSS assistance data
- Example-client `--input-disable-pipe-buffer-optimization` option
- FileInput disable pipe buffer optimization parameter
- Scheduler stream disable pipe buffer optimization option
- Scheduler stream splice() support with conditional compilation via HAVE_SPLICE

### Fixed
- Fixed GPS week/TOW wrapping for negative and overflow values
- Fixed missing includes in cxx11_compat.hpp (string, sys/stat.h)
- Fixed scheduler stream to call on_complete callback when splice returns 0 (EOF)
- Fixed Modem::enable_echo() and disable_echo() to properly wait for and consume OK responses
- Fixed Modem::set_cops() to remove trailing comma from AT+COPS command
- Fixed errno race conditions where logging calls clobbered errno before checking EINPROGRESS/EAGAIN/EWOULDBLOCK
- Example-client lpp2eph ephemeris scale factors for all GNSS (APowerHalf: 2⁻¹⁹, Crc/Crs: 2⁻⁵ for GPS/Galileo and 2⁻⁶ for BeiDou, keplerToe: 60s)
- Tokoro generator get_grid_position now correctly handles north-west reference point (latitude decreases going south)
- Tokoro CorrectionData now correctly receives correction point set pointer
- Tokoro generator now preserves existing CorrectionData when processing incremental LPP updates
- Scheduler stream pipe buffer size optimization (64MB to 1MB)
- Example-client A-GNSS processor with periodic, triggered, and both modes
- Example-client `--agnss-mode` and `--agnss-triggered-cooldown` options
- Example-client `--print agnss` option to display missing ephemeris
- Example-client `--tkr-vrs-mode=grid` and `--tkr-vrs-grid-position` for grid-based VRS
- Tokoro generator missing ephemeris tracking with missing_ephemeris() method
- Tokoro generator get_grid_position() helper for grid coordinate calculation
- MissingEphemeris struct and MissingEphemerisPrint inspector
- Example-client `--log-file` option to redirect log output to file
- Loglet file output support via `set_output_file()`
- SUPL library now uses loglet properly with function tracing and correct asserts
- A-GNSS processor prevents use-after-free by storing cell and identity by value
- LPP client prevents double-free by clearing session client pointers before destruction
- A-GNSS processor properly manages client lifetime during requests
- Example-client A-GNSS processor now copies cell data to prevent use-after-free crashes
- Example-client A-GNSS processor no longer destroys client during disconnect callback
- LPP client now clears session client pointers before destruction to prevent double-free
- Example-client A-GNSS processor now properly keeps client alive during requests
- LPP navigation model request now properly initializes reqNavList with satellite request bitmap
- LPP lpp2eph processor now correctly handles nav_KeplerianSet orbit model format
- LPP lpp2eph processor IOD extraction fixed for proper bit string parsing
- Example-client cell information parsing now works when A-GNSS is enabled
- Example-client identity parsing now works when A-GNSS is enabled without location server
- A-GNSS support via LPP with periodic requests, optional separate identity, and per-request client creation
- C++11 compatibility support via `CMAKE_CXX_STANDARD` variable
- CONSTEXPR_CXX20 macro for functions requiring C++20 constexpr support
- WIP positioning engine `idokeido`
- Input/output stage with `chain=tlf` support
- Stage `tlf` for reading and writing time-logged format data
- Function performance logging with `LOG_FUNCTION_PERFORMANCE=ON`
- Chunked-log output interface for hourly rotating log files with timestamp names
- Comprehensive logging support in modem-ctrl example with command-line flags
- Hexdump functionality for verbose logging in modem dependency
- `discard_errors` and `discard_unknowns` options to input parsers
- NMEA message ordering to prevent incomplete location reports
- Message print support via `--print` option
- Support for NMEA ending in only LF via `nmea_lf_only` option
- Support for GGA messages with only 12 values
- All compiler warnings in codebase (excluding external generated ASN.1 code)
- CORE_UNREACHABLE_CASE macro incorrectly expanding on clang due to GCC compatibility defines
- Control flow warnings in exhaustive switch statements by adding explicit CORE_UNREACHABLE() markers
- Sign conversion warnings in modem hexdump by using size_t consistently
- Enumerated/non-enumerated conditional expression warning in lpp client
- GCC 4.8 template context error by using ERRNO_FMT/ERRNO_ARGS instead of strerror directly
- Int-in-bool-context warning in RTCM datafields by extracting multiplication result
- Maybe-uninitialized warnings in RTCM extract functions by initializing Maybe value member
- Strncpy truncation warning by ensuring null termination in interface name
- CPM not being included when INCLUDE_GENERATOR_IDOKEIDO is enabled
- C++17 structured bindings in idokeido replaced with C++11 compatible iteration
- C++20 constexpr functions using std::sqrt made compatible with C++11/14 via CONSTEXPR_CXX20 macro
- SerialInput incorrectly treating set_fd return value as error
- Output assert when chain/stages not specified
- Forward task and missing data during fast reads
- Missing #ifdef guards
- Remaining set_fd calls
- NMEA processor logging issues
- Cell requirement when `--ls-disable` and `--ad-disable` are used

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
