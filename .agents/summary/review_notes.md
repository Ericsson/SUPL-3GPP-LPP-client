# Review Notes

## Consistency Check

### ✅ Consistent

- Module names match CMake target names across all files (e.g., `dependency::format::nmea` confirmed in `dependency/format/nmea/CMakeLists.txt`)
- Generator sub-module CMake flags are consistent between `codebase_info.md` and `dependencies.md`
- `datatrace` is consistently described as optional (`DATA_TRACING` flag) across all files
- LPP session states (DISCONNECTED → CONNECTED → ESTABLISHED) match `dependency/lpp/session.cpp`
- Error domain enum values match `dependency/core/include/core/error.hpp`
- Streamline `Inspector<T>` interface matches `dependency/streamline/include/streamline/inspector.hpp`

### ⚠️ Minor Inconsistencies

1. **`components.md` — loglet listed twice**: The `dependency/loglet` section appears in the main list and is also referenced as "See above" at the end. The duplicate reference should be removed on next update.

2. **`interfaces.md` — format parser API is illustrative**: The parser callback APIs shown (e.g., `parser.on_gga(...)`) are representative patterns. Actual method names should be verified against the specific parser headers before use.

3. **`dependencies.md` — `asn1::generated::fugou`**: The `external/asn.1/fugou_generated/` directory exists but its CMake target name was not verified. Treat as tentative.

## Completeness Check

### Gaps Identified

#### Format Sub-modules (Low Detail)
- `format/lpp`, `format/ctrl`, `format/helper`, `format/nav` are listed in `components.md` but have no description of their specific message types or APIs. These are less commonly used but agents working on LPP framing or control messages will need to inspect the headers directly.

#### `io/stream/` Sub-directory
- The stream adapter sub-directory (`dependency/io/stream/`) is mentioned briefly as "stream adapters (split, merge, filter)" but the specific adapter types are not documented. Agents adding new I/O pipelines should inspect `dependency/io/stream/` directly.

#### `lpp/periodic_session/`
- The periodic session handler (`dependency/lpp/periodic_session/handler.cpp`, `assistance_data.cpp`) is not described in detail. Agents working on periodic LPP delivery should read these files directly.

#### `scripts/` Directory
- Utility scripts are not documented:
  - `format.sh` — runs clang-format
  - `analyze.sh` — runs static analysis
  - `capture_rtcm_test_data.sh` — captures RTCM test data
  - `geotiff_to_cpp.py` — converts GeoTIFF to C++ arrays
  - `track_iod_changes.py` — IOD change tracking
  - `analyze_iod_example.py` — IOD analysis
  - `analyze_build_logs.py` — build log analysis

#### `plans/` Directory
- The `plans/` directory contains ~30 design documents covering planned features and refactors. These are not indexed in the documentation. Key plans include:
  - `TOKORO_TESTING_GUIDE.md` — Tokoro snapshot testing guide
  - `EPHEMERIS_REFACTOR_PLAN.md` — ephemeris refactor
  - `UNIFIED_CORRECTION_STORE.md` — correction store design
  - `TODO.md` — current task list
  - `INDEX.md` — plans index

#### `cmake/target.cmake` — `setup_target()`
- The `setup_target()` macro is referenced throughout but its contents (compiler flags, warning levels, etc.) are not documented. Agents adding new targets should inspect `cmake/target.cmake`.

#### Docker Infrastructure
- `docker/` build scripts (`build-images.py`, `build-compilers.py`, `build-options.py`) are not documented. These are used for multi-platform Docker image builds.

#### `CONTROL.md` and `UPGRADE_FROM_V3.md`
- `CONTROL.md` (project control/governance) and `UPGRADE_FROM_V3.md` (v3→v4 migration guide) are not referenced in the documentation.

### Language Support Limitations

- The ASN.1 generated code (`external/asn.1/lpp_generated/`, `supl_generated/`) is C, not C++. This is intentional (asn1c generates C). The documentation correctly notes this.
- Python scripts in `docker/` and `scripts/` are not analyzed in depth.

## Recommendations

1. **Add `plans/INDEX.md` reference** to `index.md` so agents know where to find design documents.
2. **Document `setup_target()` behavior** in `codebase_info.md` (warning flags, sanitizer options).
3. **Add `scripts/` section** to `codebase_info.md` listing available utility scripts.
4. **Verify `asn1::generated::fugou` target name** against `external/asn.1/CMakeLists.txt`.
5. **Expand `format/lpp` and `format/ctrl`** descriptions when agents need to work with those parsers.
