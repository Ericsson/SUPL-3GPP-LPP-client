# Dependencies

## External Dependencies

### Eigen3

- **Source**: Fetched via CPM (CMake Package Manager) at configure time
- **Config**: `external/eigen3.cmake`
- **Usage**: Linear algebra (Vector3d, Matrix3d) in `coordinates`, `ephemeris`, `generator/tokoro`
- **Version**: Latest compatible with C++11/20

### args (taywee/args)

- **Source**: Vendored in `external/args/include/args.hxx`
- **Config**: `external/args.cmake`, `external/args/CMakeLists.txt`
- **Usage**: CLI argument parsing in all example applications
- **License**: MIT

### ASN.1 Codec (asn1c-generated)

- **Source**: Vendored in `external/asn.1/`
- **Config**: `external/asn.1/CMakeLists.txt`
- **Targets**:
  - `asn1::generated::lpp` — LPP ASN.1 generated C code (`lpp_generated/`)
  - `asn1::generated::supl` — SUPL ASN.1 generated C code (`supl_generated/`)
  - `asn1::skeleton` — asn1c runtime skeleton (`skeleton/`)
  - `asn1::helper` — helper utilities (`helper/`)
  - `asn1::generated::fugou` — additional generated code (`fugou_generated/`)
- **License**: BSD (asn1c)

### OpenSSL / libssl

- **Source**: System package (`libssl-dev`)
- **Usage**: TLS for SUPL TCP connections in `dependency/supl/tcp_client.cpp`
- **Install**: `sudo apt install libssl-dev`

## System Dependencies

| Dependency | Purpose | Install |
|-----------|---------|---------|
| `g++` / `clang++` | C++ compiler | `sudo apt install g++` |
| `cmake` 3.14+ | Build system | `sudo apt install cmake` |
| `ninja-build` | Build backend | `sudo apt install ninja-build` |
| `libssl-dev` | TLS support | `sudo apt install libssl-dev` |
| `clang` + libFuzzer | Fuzzing (optional) | `sudo apt install clang` |

## Internal Dependency Graph

```mermaid
graph TD
    core["dependency::core"]
    loglet["dependency::loglet"]
    gnss["dependency::gnss"]
    time["dependency::time"]
    coords["dependency::coordinates"]
    eph["dependency::ephemeris"]
    maths["dependency::maths"]
    msgpack["dependency::msgpack"]
    io["dependency::io"]
    scheduler["dependency::scheduler"]
    streamline["dependency::streamline"]
    modem["dependency::modem"]
    format["dependency::format::*"]
    supl["dependency::supl"]
    lpp["dependency::lpp"]
    gen_rtcm["generator::rtcm"]
    gen_spartn["generator::spartn"]
    gen_tokoro["generator::tokoro"]
    gen_idokeido["generator::idokeido"]
    datatrace["dependency::datatrace"]
    asn1_lpp["asn1::generated::lpp"]
    asn1_supl["asn1::generated::supl"]
    eigen3["Eigen3"]

    loglet --> core
    gnss --> core
    time --> core
    coords --> core
    coords --> eigen3
    eph --> core
    eph --> gnss
    eph --> time
    eph --> coords
    maths --> core
    msgpack --> core
    io --> core
    io --> loglet
    scheduler --> core
    scheduler --> loglet
    streamline --> core
    streamline --> loglet
    streamline --> scheduler
    modem --> core
    modem --> loglet
    modem --> scheduler
    format --> core
    format --> gnss
    format --> loglet
    supl --> core
    supl --> loglet
    supl --> scheduler
    supl --> asn1_supl
    lpp --> core
    lpp --> time
    lpp --> supl
    lpp --> scheduler
    lpp --> loglet
    lpp --> asn1_lpp
    gen_rtcm --> lpp
    gen_rtcm --> gnss
    gen_rtcm --> format
    gen_spartn --> lpp
    gen_spartn --> gnss
    gen_tokoro --> gen_rtcm
    gen_tokoro --> eph
    gen_tokoro --> coords
    gen_tokoro --> time
    gen_tokoro --> maths
    gen_idokeido --> lpp
    gen_idokeido --> eph
    datatrace --> core
    datatrace --> loglet
    datatrace --> msgpack
```

## Optional Feature Dependencies

| Feature | Required Modules | CMake Flag |
|---------|-----------------|-----------|
| RTCM generation | `generator::rtcm` | `INCLUDE_GENERATOR_RTCM=ON` |
| SPARTN generation | `generator::spartn` | `INCLUDE_GENERATOR_SPARTN=ON` |
| Tokoro OSR | `generator::tokoro` + RTCM | `INCLUDE_GENERATOR_TOKORO=ON` |
| Idokeido PPP | `generator::idokeido` | `INCLUDE_GENERATOR_IDOKEIDO=ON` |
| RINEX output | `format::rinex` + RTCM | `INCLUDE_FORMAT_RINEX=ON` |
| ANTEX parsing | `format::antex` + RTCM | `INCLUDE_FORMAT_ANTEX=ON` |
| Data tracing | `datatrace` | `DATA_TRACING=ON` |

## Cross-Compilation Toolchains

Toolchain files in `cmake/`:

| File | Target |
|------|--------|
| `toolchain-arm.cmake` | Generic ARM |
| `toolchain-aarch64.cmake` | AArch64 |
| `toolchain-rpi-armv6.cmake` | Raspberry Pi ARMv6 |
| `toolchain-rpi-aarch64.cmake` | Raspberry Pi AArch64 |
| `toolchain-beaglebone.cmake` | BeagleBone |

Docker images for cross-compilation are in `docker/linux/`.
