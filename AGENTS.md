# Agent Guidelines for SUPL-3GPP-LPP-client

## Project Overview

C++ toolkit (v4.0.23) for 3GPP LPP (LTE Positioning Protocol) clients with SUPL communication support. Provides libraries for positioning protocols, message converters (LPP→RTCM/SPARTN), and example applications.

**Stack**: C++11/C++20 · CMake 3.14+ · Ninja · Eigen3 · asn1c-generated ASN.1 codec

## Directory Map

```
dependency/          # Library modules (each is a CMake static lib)
  core/              # Base types, error handling, macros
  coordinates/       # ECEF/LLH/ENU/NED/AER (type-safe frame-parameterized)
  gnss/              # SatelliteId, SignalId
  time/              # GPS/UTC/TAI/BDT/GST/GLO time systems
  ephemeris/         # GPS/GAL/BDS/GLO/QZS ephemeris computation
  format/            # Parsers: nmea · ubx · rtcm · lpp · at · antex · ctrl · nav · rinex
  generator/         # Generators: rtcm · spartn · tokoro · idokeido (all optional)
  io/                # Serial · TCP · UDP · File · stdin/stdout + stream adapters
  scheduler/         # epoll event loop (Scheduler, Timeout, Periodic, Stream)
  lpp/               # LPP client, sessions, assistance data, capabilities
  supl/              # SUPL encode/decode, TCP/TLS client
  streamline/        # Typed pub/sub pipeline (System, Inspector<T>, Consumer<T>)
  loglet/            # Structured logging with module-scoped levels
  modem/             # AT-command modem interface
  maths/             # mat3, float3
  msgpack/           # MessagePack serialization
  datatrace/         # Optional data tracing (DATA_TRACING=ON)
examples/            # Binaries: client · ntrip · transform · lpp2spartn · modem_ctrl · crs · ctrl_toggle · relay · tbin-merge · tbin-parse · tokoro-post
external/            # Vendored: asn.1 (lpp_generated, supl_generated) · args (CLI) · eigen3 (CPM)
tests/               # doctest unit tests + libFuzzer fuzz targets
cmake/               # CMake modules, toolchains, setup_target()
scripts/             # format.sh · analyze.sh · capture_rtcm_test_data.sh · geotiff_to_cpp.py
plans/               # Design documents (see plans/INDEX.md)
docker/              # Multi-platform Docker build infrastructure
```

## Key CMake Flags

| Flag | Description |
|------|-------------|
| `INCLUDE_GENERATOR_RTCM` | Enable RTCM generator |
| `INCLUDE_GENERATOR_SPARTN` | Enable SPARTN generator |
| `INCLUDE_GENERATOR_TOKORO` | Enable Tokoro OSR generator (requires RTCM) |
| `INCLUDE_GENERATOR_IDOKEIDO` | Enable Idokeido PPP generator |
| `INCLUDE_FORMAT_RINEX` | Enable RINEX builder (requires RTCM generator) |
| `INCLUDE_FORMAT_ANTEX` | Enable ANTEX parser (requires RTCM generator) |
| `DATA_TRACING` | Enable datatrace module |
| `BUILD_EXAMPLES` | Build example applications |
| `BUILD_TESTING` | Build tests |
| `ENABLE_FUZZING` | Enable libFuzzer targets (requires Clang) |
| `CMAKE_CXX_STANDARD` | Set to `11` for C++11 compat mode |

## Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes/Structs | PascalCase | `SatelliteId`, `EnuBasis` |
| Functions/Methods | snake_case | `ecef_to_llh`, `from_degrees` |
| Member Variables | mPascalCase | `mGnss`, `mScheduler` |
| Constants | UPPER_SNAKE_CASE | `GPS_ABS_COUNT` |
| Namespaces | lowercase | `coordinates`, `ts`, `generator` |
| Template Parameters | PascalCase | `Frame`, `DataType` |

## Repo-Specific Patterns

### CMake Module Pattern
Every `dependency/<name>` follows:
```cmake
add_library(dependency_<name> STATIC <sources>)
add_library(dependency::<name> ALIAS dependency_<name>)
target_include_directories(dependency_<name> PUBLIC include/)
target_link_libraries(dependency_<name> PUBLIC <deps>)
setup_target(dependency_<name>)  # applies consistent compiler flags
```
Add new libraries to `dependency/CMakeLists.txt`.

### Streamline Processor Pattern
Data processors inherit from `streamline::Inspector<T>` (read-only) or `streamline::Consumer<T>` (exclusive):
```cpp
class MyProcessor : public streamline::Inspector<MyDataType> {
    void inspect(streamline::System& sys, MyDataType const& data, uint64_t tag) override;
    char const* name() const noexcept override { return "MyProcessor"; }
};
// Register: system.add_inspector<MyProcessor>(/* ctor args */);
```
Examples: `Lpp2Rtcm`, `Lpp2Spartn`, `NmeaProcessor`, `TokoroProcessor`.

### Template `_impl` Pattern
Non-template implementation functions use `_impl` suffix to reduce compile times:
```cpp
// Header: declare impl + template wrapper
Vector3d ecef_to_llh_impl(Vector3d const& ecef, Ellipsoid const& e);
template<typename Frame>
Llh<Frame> ecef_to_llh(Ecef<Frame> const& ecef) {
    return Llh<Frame>{ecef_to_llh_impl(ecef.value, Frame::ellipsoid)};
}
```

### Error Handling
No exceptions. Return `Error` or `Result<T>`:
```cpp
NODISCARD Result<int> my_func() {
    if (bad) return make_error(ErrorDomain::IO, MY_CODE);
    return 42;
}
```
Use `Maybe<T>` (not `std::optional`) for optional values — required for C++11 compat.

### Coordinate Types
Frame-parameterized structs prevent mixing reference frames at compile time:
```cpp
coordinates::Ecef<WGS84Frame> ecef{x, y, z};
coordinates::Llh<WGS84Frame> llh = coordinates::ecef_to_llh(ecef);
```

### Logging
```cpp
LOGLET_MODULE("my_module");  // file scope
DEBUGF("val=%d", x);
INFOF("connected to %s", host);
WARNF("retry %d", n);
ERRORF("failed: %s", err.code_name());
VSCOPE_FUNCTION();  // verbose entry/exit tracing
```

### C++11 Compatibility
When `USE_CXX11` is set (i.e., `CMAKE_CXX_STANDARD=11`):
- Use `Maybe<T>` not `std::optional`
- Use `const char*` or `std::string const&` not `std::string_view`
- Use macros from `core/core.hpp` (`NODISCARD`, `NOEXCEPT`, `CONSTEXPR`, etc.)
- Check `USE_CXX11` flag before using C++14+ features

## Code Generation Rules

1. Write only necessary code — no verbosity
2. No comments unless code is genuinely hard to understand
3. No tests unless explicitly requested; never modify existing tests unless asked
4. Return `Error`/`Maybe<T>` — never throw exceptions
5. Mark methods `const` when they don't modify state
6. Use `NODISCARD` on functions whose return value must not be ignored
7. Public headers in `include/<module>/`; one class per file
8. Do not edit `CHANGELOG.md`; the PR title (conventional commit style, e.g. `fix(nmea): ...`) and description are the changelog entry and feed the GitHub release notes

## Custom Instructions
<!-- This section is for human and agent-maintained operational knowledge.
     Add repo-specific conventions, gotchas, and workflow rules here.
     This section is preserved exactly as-is when re-running codebase-summary. -->

- `main` is protected: never push to it. All changes go through a PR from a feature branch.
- Only create branches, commits, pushes, tags, PRs, or releases when explicitly asked. Never force-push, `reset --hard`, or otherwise rewrite history.
- Release flow: release PR (bump `CLIENT_VERSION`/`CLIENT_VERSION_INT` in `CMakeLists.txt`, README badge, and `LAST_UPDATE_CHECKED` in `dependency/time/timestamp.cpp` after confirming the leap-second count) -> merge -> full clean build + `ctest` -> tag the merge commit `vX.Y.Z` -> `gh release create vX.Y.Z --generate-notes` -> `docker/build-images.py --push --tag vX.Y.Z --latest` (builders are pulled from Rosetta)
