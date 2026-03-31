# Codebase Info

## Project

- **Name**: SUPL-3GPP-LPP-client
- **Version**: 4.0.23
- **License**: MXM (see LICENSE.txt)
- **Languages**: C++11/C++20 (configurable), C (ASN.1 generated code)
- **Build System**: CMake 3.14+ with Ninja
- **Standard**: C++20 by default; C++11 compatibility mode via `-DCMAKE_CXX_STANDARD=11`

## Directory Layout

```
/
├── dependency/          # Core library modules (16 modules)
│   ├── core/            # Base types, error handling, macros
│   ├── coordinates/     # Coordinate transformations
│   ├── gnss/            # GNSS satellite/signal types
│   ├── time/            # Time system conversions
│   ├── ephemeris/       # Satellite ephemeris calculations
│   ├── format/          # Protocol parsers (nmea, ubx, rtcm, lpp, at, antex, ctrl, nav, rinex)
│   ├── generator/       # Message generators (rtcm, spartn, tokoro, idokeido)
│   ├── io/              # I/O abstractions (serial, TCP, UDP, file, stdin/stdout)
│   ├── scheduler/       # epoll-based event loop
│   ├── lpp/             # LPP protocol implementation
│   ├── supl/            # SUPL protocol implementation
│   ├── streamline/      # Typed pub/sub pipeline
│   ├── loglet/          # Logging framework
│   ├── modem/           # AT-command modem interface
│   ├── maths/           # Math utilities (mat3, float3)
│   ├── msgpack/         # MessagePack serialization
│   ├── ephemeris/       # Ephemeris (GPS, GAL, BDS, GLO, QZS)
│   └── datatrace/       # Optional data tracing (DATA_TRACING flag)
├── examples/            # Example applications
│   ├── client/          # Full-featured LPP client demo
│   ├── ntrip/           # NTRIP client demo
│   ├── transform/       # Coordinate transform demo
│   ├── lpp2spartn/      # LPP-to-SPARTN converter
│   ├── modem_ctrl/      # Modem control utility
│   ├── crs/             # CRS utility
│   └── ctrl_toggle/     # Control toggle utility
├── external/            # Vendored dependencies
│   ├── asn.1/           # ASN.1 codec (lpp_generated, supl_generated, skeleton)
│   ├── args/            # CLI argument parsing (args.hxx)
│   └── eigen3.cmake     # Eigen3 (fetched via CPM)
├── tests/               # Unit tests (doctest) + fuzz tests (libFuzzer)
├── cmake/               # CMake modules and toolchains
├── docker/              # Docker build infrastructure
├── scripts/             # Utility scripts (format, analyze, capture)
└── plans/               # Design documents and feature plans
```

## Key CMake Flags

| Flag | Default | Description |
|------|---------|-------------|
| `CMAKE_CXX_STANDARD` | 20 | C++ standard (11 or 20) |
| `BUILD_EXAMPLES` | OFF | Build example applications |
| `BUILD_TESTING` | OFF | Build tests |
| `ENABLE_FUZZING` | OFF | Enable libFuzzer targets |
| `INCLUDE_GENERATOR_RTCM` | OFF | Build RTCM generator |
| `INCLUDE_GENERATOR_SPARTN` | OFF | Build SPARTN generator |
| `INCLUDE_GENERATOR_TOKORO` | OFF | Build Tokoro generator (requires RTCM) |
| `INCLUDE_GENERATOR_IDOKEIDO` | OFF | Build Idokeido generator |
| `INCLUDE_FORMAT_RINEX` | OFF | Build RINEX format (requires RTCM generator) |
| `INCLUDE_FORMAT_ANTEX` | OFF | Build ANTEX format (requires RTCM generator) |
| `DATA_TRACING` | OFF | Enable datatrace module |
| `ENABLE_CLANG_TIDY` | OFF | Enable clang-tidy |
| `ENABLE_CPPCHECK` | OFF | Enable cppcheck |
| `ENABLE_IWYU` | OFF | Enable include-what-you-use |

## Static Analysis Config

- `.clang-tidy` — clang-tidy rules at repo root
- `.clang-format` — code formatting rules
- `cmake/iwyu.imp` — IWYU mapping file
- `external/.clang-tidy` — suppresses warnings in vendored code
