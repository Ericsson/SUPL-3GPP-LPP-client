# Components

## dependency/core

Foundation library. No dependencies on other modules.

- `core/core.hpp` — Portability macros: `NODISCARD`, `NOEXCEPT`, `OVERRIDE`, `EXPLICIT`, `CONSTEXPR`, `CORE_ASSERT`, `CORE_UNREACHABLE`
- `core/error.hpp` — `Error` struct with `ErrorDomain` enum; `make_error()`, `ok()`; domain registration via `REGISTER_ERROR_DOMAIN`
- `core/result.hpp` — `Result<T>` wrapping a value or `Error`
- `core/logging.hpp` — Scope/function tracing helpers (`VSCOPE_FUNCTION`, `FUNCTION_SCOPE`)
- `core/generic.hpp` — `Maybe<T>` (C++11-compatible optional)
- `cxx11_compat.hpp` — C++11 compatibility shims

## dependency/loglet

Structured logging with module-scoped levels.

- `LOGLET_MODULE("name")` — declares a module
- `DEBUGF`, `INFOF`, `WARNF`, `ERRORF`, `VERBOSEF` — level-gated printf-style macros
- `VSCOPE_FUNCTION()` / `VSCOPE_FUNCTIONF(fmt, ...)` — verbose scope tracing
- Runtime level control per module; optional color output; optional file output

## dependency/scheduler

epoll-based single-threaded event loop.

- `Scheduler` — main event loop; `run()`, `cancel()`, `defer(cb)`, `schedule(task)`
- `FileDescriptor` — wraps an fd with read/write/error callbacks
- `Socket` — TCP/Unix socket with connect/listen/accept
- `Timeout` — one-shot timer
- `Periodic` — repeating timer
- `Stream` — buffered read/write over a `FileDescriptor`

## dependency/io

I/O abstractions built on top of scheduler.

- `Input` / `Output` — abstract read/write interfaces
- `TcpClient` / `TcpServer` — TCP stream I/O
- `UdpClient` / `UdpServer` — UDP datagram I/O
- `SerialPort` — serial port with baud/parity/stop-bit config
- `FileStream` — file-backed I/O
- `StdinStream` / `StdoutStream` — standard streams
- `Buffer` / `WriteBuffer` — in-memory buffering
- `Registry` — named I/O endpoint registry
- `stream/` — stream adapters (split, merge, filter)

## dependency/streamline

Typed publish/subscribe pipeline backed by the scheduler.

- `System` — type-indexed queue manager; `push<T>()`, `add_inspector<I>()`, `add_consumer<C>()`
- `Inspector<T>` — abstract base for read-only data processors; override `inspect()`
- `Consumer<T>` — abstract base for exclusive consumers; override `consume()`
- `Producer<T>` — helper for pushing data into the system
- `Queue<T>` / `QueueTask<T>` — internal typed queue with scheduler integration

## dependency/gnss

GNSS satellite and signal type system.

- `SatelliteId` — typed satellite identifier (system + PRN); supports GPS, GLONASS, Galileo, BeiDou, QZSS, SBAS, NavIC
- `SignalId` — typed signal identifier (satellite + band/frequency)
- Phase alignment utilities

## dependency/time

Multi-system time representations and conversions.

- `GpsTime` — GPS week + time-of-week
- `UtcTime` — UTC with leap second handling
- `TaiTime` — International Atomic Time
- `BdtTime` — BeiDou Time
- `GstTime` — Galileo System Time
- `GloTime` — GLONASS time
- `Timestamp` — generic high-resolution timestamp
- Conversion functions between all systems

## dependency/coordinates

Type-safe coordinate transformations using Eigen3.

- `Ecef<Frame>` — Earth-Centered Earth-Fixed
- `Llh<Frame>` — Latitude/Longitude/Height
- `Enu<Frame>` — East/North/Up local frame
- `Ned<Frame>` — North/East/Down local frame
- `Aer` — Azimuth/Elevation/Range
- `Ellipsoid` — ellipsoid parameters (WGS84, GRS80, etc.)
- `NullReferenceFrame` — default frame when no specific frame needed
- Conversion functions: `ecef_to_llh`, `llh_to_ecef`, `ecef_to_enu`, `enu_to_ecef`, etc.

## dependency/ephemeris

Satellite ephemeris computation.

- `GpsEphemeris` — GPS Keplerian ephemeris → ECEF position/velocity/clock
- `GalileoEphemeris` — Galileo ephemeris
- `BdsEphemeris` — BeiDou ephemeris
- `GloEphemeris` — GLONASS numerical integration ephemeris
- `QzssEphemeris` — QZSS ephemeris
- Common `Ephemeris` base with `compute_position()` interface

## dependency/format

Protocol parsers. Each sub-module is a separate CMake target.

| Sub-module | Target | Description |
|-----------|--------|-------------|
| `nmea` | `dependency::format::nmea` | NMEA 0183 sentence parser |
| `ubx` | `dependency::format::ubx` | u-blox UBX binary parser |
| `rtcm` | `dependency::format::rtcm` | RTCM 3.x message parser + data types |
| `lpp` | `dependency::format::lpp` | LPP message framing/parsing |
| `at` | `dependency::format::at` | AT command parser |
| `antex` | `dependency::format::antex` | ANTEX antenna calibration parser |
| `ctrl` | `dependency::format::ctrl` | Control message format |
| `nav` | `dependency::format::nav` | Navigation message parser (GPS LNAV) |
| `rinex` | `dependency::format::rinex` | RINEX observation file builder |
| `helper` | `dependency::format::helper` | Shared parsing utilities |

## dependency/generator

Message generators (LPP assistance data → correction formats). All are optional CMake targets.

| Sub-module | CMake Flag | Description |
|-----------|-----------|-------------|
| `rtcm` | `INCLUDE_GENERATOR_RTCM` | LPP → RTCM MSM7 + ephemeris messages |
| `spartn` | `INCLUDE_GENERATOR_SPARTN` | LPP → SPARTN OCB + HPAC messages |
| `tokoro` | `INCLUDE_GENERATOR_TOKORO` | SSR-based OSR correction generator (requires RTCM) |
| `idokeido` | `INCLUDE_GENERATOR_IDOKEIDO` | PPP correction generator |

## dependency/supl

SUPL (Secure User Plane Location) protocol implementation.

- `Session` — SUPL session state machine (POSINIT, POS, END)
- `TcpClient` — TLS/TCP connection to SUPL server
- `encode.cpp` — SUPL message encoding (UPER)
- `decode.cpp` — SUPL message decoding
- `print.cpp` — SUPL message pretty-printer

## dependency/lpp

LPP (LTE Positioning Protocol) client implementation.

- `Client` — top-level LPP client; manages sessions, capabilities, assistance data requests
- `Session` — per-connection LPP session state machine
- `SingleSession` — one-shot assistance data session
- `PeriodicSession` — periodic assistance data delivery
- `Transaction` — LPP transaction tracking
- `RequestAssistanceData` — builds LPP RequestAssistanceData messages
- `ProvideCapabilities` — builds LPP ProvideCapabilities messages
- `ProvideLocationInformation` — builds LPP ProvideLocationInformation messages
- `LocationInformation` — typed location data structures

## dependency/modem

AT-command modem interface for cellular connectivity.

- `Modem` — sends AT commands, parses responses, manages modem state
- Supports cell identity queries (MCC/MNC/TAC/CellId) for SUPL positioning

## dependency/maths

Math utilities.

- `mat3` — 3×3 matrix operations
- `float3` — 3-component float vector

## dependency/msgpack

MessagePack binary serialization for test data and snapshots.

- `MsgpackReader` / `MsgpackWriter` — read/write MessagePack streams
- Used for ephemeris test data and Tokoro snapshot testing

## dependency/loglet

See above under core support libraries.

## dependency/datatrace

Optional data tracing module (enabled with `-DDATA_TRACING=ON`). Records correction data for offline analysis.

## examples/client

Full-featured LPP client application (`example-client`).

- `main.cpp` — CLI parsing, system setup, pipeline construction
- `config/` — input/output/stream/UBX configuration parsers
- `processor/` — data processors: Tokoro, Idokeido, NMEA output, LPP→ephemeris, POSSIB logger
- `io.cpp` — I/O endpoint setup (serial, TCP, UDP, file)
- `tag_registry.cpp` — tag-based message routing

## examples/ntrip

NTRIP client that connects to an NTRIP caster and forwards corrections to a GNSS receiver.

## examples/transform

Coordinate transformation demo.

## examples/lpp2spartn

Standalone LPP-to-SPARTN converter.

## examples/modem_ctrl

Modem control utility for AT-command interaction.

## examples/crs

CRS (Coordinate Reference System) utility.

## examples/ctrl_toggle

Control toggle utility for runtime feature switching.
