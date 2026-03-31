# Knowledge Base Index

## How to Use This Documentation

This index is the primary entry point for AI assistants navigating the SUPL-3GPP-LPP-client codebase. Load this file into context first, then consult specific files based on the task at hand.

### Quick Reference

| Question | File to Consult |
|----------|----------------|
| What does this project do? | `codebase_info.md` + `architecture.md` |
| Where is X module located? | `components.md` |
| How do I use the LPP client API? | `interfaces.md` |
| What data types are used? | `data_models.md` |
| How does the LPP session flow work? | `workflows.md` |
| What are the external dependencies? | `dependencies.md` |
| What CMake flags exist? | `codebase_info.md` |
| How does the streamline pipeline work? | `architecture.md` + `interfaces.md` |
| How do I add a new processor? | `components.md` (streamline section) + `interfaces.md` |
| What error types exist? | `data_models.md` |
| How are coordinates handled? | `data_models.md` + `interfaces.md` |
| How does the build system work? | `codebase_info.md` + `dependencies.md` |

## File Summaries

### `codebase_info.md`
Project metadata, directory layout, CMake flags table, and static analysis configuration. Start here to understand the project structure and available build options.

### `architecture.md`
System architecture diagrams showing the layered design, component relationships, key design patterns (Streamline pipeline, Error handling, Coordinate frames, LPP session state machine), and the CMake module pattern. Essential for understanding how components fit together.

### `components.md`
Detailed description of every module in `dependency/`, `examples/`, and their responsibilities. Covers public APIs, key types, and what each module does. Use this to locate where specific functionality lives.

### `interfaces.md`
Public API reference for all major modules: LPP client, SUPL, scheduler, I/O, streamline pipeline, coordinates, GNSS types, time, ephemeris, generators, format parsers, logging, and error handling. Use this when writing code that uses these libraries.

### `data_models.md`
Data structure definitions with class diagrams: Error/Result types, GNSS satellite/signal types, coordinate types, time types, LPP location information, ephemeris types, RTCM data types, SPARTN structures, and MessagePack serialization. Use this to understand what data flows through the system.

### `workflows.md`
Sequence and flow diagrams for key processes: LPP assistance data request, periodic delivery, LPP→RTCM conversion pipeline, Tokoro SSR→OSR generation, example-client startup, format parser data flow, build workflow, test execution, and fuzzing. Use this to understand runtime behavior.

### `dependencies.md`
External dependencies (Eigen3, args, ASN.1 codec, OpenSSL), system requirements, internal dependency graph, optional feature dependencies, and cross-compilation toolchains. Use this when setting up the build or understanding module coupling.

### `review_notes.md`
Consistency and completeness review findings, identified gaps, and recommendations.

## Codebase Quick Facts

- **Version**: 4.0.23
- **Language**: C++20 (C++11 compat mode available)
- **Build**: CMake + Ninja
- **Test framework**: doctest
- **Fuzzing**: libFuzzer (nmea, ubx, rtcm, at parsers)
- **Key pattern**: Streamline typed pub/sub pipeline for data processing
- **Error handling**: `Error`/`Result<T>` — no exceptions
- **Optional types**: `Maybe<T>` (not `std::optional` for C++11 compat)
- **Logging**: `LOGLET_MODULE` + `DEBUGF`/`INFOF`/`WARNF`/`ERRORF`/`VERBOSEF`
- **Coordinates**: Type-safe frame-parameterized structs (`Ecef<Frame>`, `Llh<Frame>`, etc.)

## Navigation Tips for AI Assistants

1. **Finding a module's public API**: Look in `dependency/<module>/include/<module>/`
2. **Finding implementation**: Look in `dependency/<module>/*.cpp`
3. **Adding a new processor**: Inherit from `streamline::Inspector<T>`, implement `inspect()` and `name()`
4. **Adding a new library**: Follow the CMake pattern in `components.md`, add to `dependency/CMakeLists.txt`
5. **Understanding LPP message flow**: See `workflows.md` sequences 1 and 2
6. **Understanding correction generation**: See `workflows.md` sequence 3 and 4
7. **C++11 compatibility**: Check `USE_CXX11` flag; use `Maybe<T>` not `std::optional`; use macros from `core/core.hpp`
