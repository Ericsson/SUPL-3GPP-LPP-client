# Tokoro TODO

## Bugs

- [x] `data/set.cpp` `array_to_index`: `1ULL << (64 - 1 - i)` is UB when the grid has more than 64 points (`numberOfSteps*` are 0..63 but the bitmask is SIZE(64)). Treat points beyond bit 63 as invalid.
- [x] `data/gather.cpp` gridded init: when `array_to_index` fails, an uninitialized `CorrectionPointInfo` is passed to `add_point`, which indexes `grid_points[absolute_index]` with garbage. Skip the point instead.
- [x] `examples/client/processor/tokoro.cpp` periodic callback: `ASSERT(mSystem)` aborts if a time-step tick fires before the first `inspect()`. Skip the tick until `mSystem` is set.
- [x] `examples/client/config/tokoro.cpp`: help choices missing `grid` (`--tkr-vrs-mode`) and `time-step-last` (`--tkr-gen`).

- [x] `data/grid.hpp` `point_from_array_index` matched unused (masked) slots, whose `array_index` defaults to 0. With a masked first point, `gridList[0]` was written to the masked slot and the real first point never got data.
- [x] `data/gather.cpp` grid init built points from the first gridded message's `gridList` (skipping null entries) instead of from the correction point set; now every enabled point is added.
- [x] `decode.cpp` `epoch_time_r15`: QZSS time ID fell through to `Tai::now()`. QZST is aligned to GPS time.

## Risks

- [x] Orbit/clock max-age measured from `t0 = epoch + update_interval/2` instead of the SSR epoch, unlike biases/STEC/grid. Now uses `epoch_time` (`data/correction.cpp`). `t0` itself is correct per TS 37.355 NOTE 3.
- [x] `data/ionosphere.cpp`: grid residual failure silently fell back to polynomial-only STEC. Now both polynomial and grid residual are required (`no_grid`/`grid_outside`/`grid_missing_sat`/`grid_stale`); `--tkr-iono-poly-only` (client) / `--iono-poly-only` (tokoro-post) allows the polynomial alone. Diag `STEC_GRID` shows `-` when the residual is missing.
- [x] `decode.cpp`: latitude/longitude step resolution constants were swapped (harmless, both 0.01).
- [x] GPS `lpp_iod`: client RTCM 1019 path (`processor/tokoro.cpp`) used IODE; TS 37.355 defines it as IODC, as every other path does. The 8-bit compare in `generator.cpp` find_ephemeris stays as a datafeed workaround; the ephemeris validity window prevents IODC aliasing.
- [x] `observation.hpp`: `mPhaseRange`, `mPhaseRangeRate`, `mCodeRange` now default-initialized.
- [x] `satellite.cpp` `compute_true_position`: orbit-correction failure only warned and continued with the uncorrected broadcast orbit (or a zero position inside the emission-time iteration). Now returns false, disabling the satellite with `position_failed`.
- [x] `data/set.hpp`: removed unused `latitude_min/max`, `longitude_min/max`; they assumed the reference point is the south-west corner, but TS 37.355 defines it as the north-west corner.
- [ ] `models/troposphere_models.cpp` `height_scaling()`: scales from ellipsoidal height 0. TS 37.355 does not state the reference surface for gridded tropo; SPARTN uses zero ellipsoidal height. Consistent with that assumption; confirm with the SSR provider.
- [ ] Only `GNSS-SSR-ArrayOfCorrectionPoints-r16` is supported; `listOfCorrectionPoints` and r18 `ArrayOfGridPoints` (bog16/64/256) are not.

Investigated, not issues:
- `satellite.cpp` emission-time iteration: fixed-point contraction is ~v/c per step, so 10 iterations always converge; the missing break only costs cycles.
- `satellite.cpp` clock lookup depending on the orbit lookup: both are called in a fixed order inside `update()`.
- SSR scale factors in `decode.hpp` (orbit, clock C0-C2, code/phase bias, STEC C00-C11, residual b7/b16, tropo, reference point, step) match TS 37.355 v18.4.0.
- Grid bilinear interpolation, row-major indexing and the NW-origin layout in `grid.cpp`/`set.cpp` match the spec.

## Improvements

- [ ] `data/grid.cpp` `find_top_left`: a position exactly on the outer south/east grid edge (e.g. the SE corner) fails the containment test from degree/radian round-off; add a small tolerance.
- [ ] `generator.cpp` `find_corrections`: QZSS gnss-ID assistance data is skipped as unsupported, so QZSS satellites can never get SSR corrections even though `generate_qzs` exists.
- [ ] `data/gather.cpp`: no check that `gridList` count equals the number of enabled bitmask points; with `--tkr-ignore-bitmask` and a really masked grid, entries shift onto the wrong points silently.

- [ ] Grid residuals/tropo per point are never cleared when a later gridded message omits a satellite or point; with max ages disabled (client default) a cell can mix epochs indefinitely.
- [ ] Grid is only rebuilt when `correctionPointSetID` changes; a new correction point set with the same ID but different geometry/bitmask keeps the old layout.
- [ ] `data/gather.cpp`: `TOKORO_GRID_DUMP` calls `getenv` per residual and never closes the file.
- [ ] `decode.cpp` `epoch_time_r15`: SBAS/NavIC time IDs fall back to `Tai::now()`; should reject instead.

- [ ] `data/grid.cpp` `find_top_left`: interpolation requires all four cell corners, and the top-left must be valid. A VRS in a cell with one masked corner is reported as outside the grid; consider 3-point or nearest-point fallback.
- [ ] `data/grid.cpp`: no handling for grids crossing the ±180° meridian.
- [ ] `observation.cpp` `compute_ranges`: phase-range rate holds STEC, tropo and windup constant between the two states; only geometry and clock contribute.
- [ ] Front-end parity: QZSS hard-coded off in client but configurable in tokoro-post; `--tkr-no-glonass` exists in client but GLONASS generation is unimplemented (`generator.cpp` satellite list, `find_ephemeris`).
- [ ] Deduplicate RTCM→ephemeris conversion (`processor/tokoro.cpp`, `processor/rtcm2eph.cpp`, `tokoro-post/main.cpp`) and option validation between `examples/client` and `examples/tokoro-post`.
- [ ] `diag.cpp`: log when `fopen`/`mkdir` fails; simplify `mkdirs()`.
- [ ] `satellite.cpp` `update()`: per-epoch diagnostic string building even when diagnostics are disabled.
- [ ] `generator.cpp:94`: satellite reserve is 127, actual count is 141.
- [ ] `models/helper.hpp` `range_time_division`: single-step overflow fix-up; use a loop or assert.
- [ ] Remove dead code: `GridIndex`, `IonosphereGrid*`, `TroposphereGrid*`, `IonosphericCorrection::vtec_*_residual`, `#if 0` blocks in `satellite.cpp`.
- [ ] `data/grid.hpp` `point_from_array_index`: O(n) scan.
- [ ] GLONASS FDMA channel not modeled in `Observation` (needed before GLONASS support).
- [ ] Dynamic VRS: no diagnostic when location updates go stale.
- [ ] Tests: iono grid requirement, orbit/clock age checks, config CLI parsing.
