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

- `models/helper.hpp` `range_time_division`: the fractional ms is in [0, 1), so at most one carry is possible; the single-step fix-up is sufficient.
- GLONASS and QZSS generation are off by default by design; settings left unchanged.

## Fixed (round 3)

- [x] `data/grid.cpp`: cell lookup rewritten to compute the cell from the reference point and steps instead of scanning for a valid top-left. Positions on the outer edge (including grid corners) are accepted with a 1e-6 step tolerance, and a position on a shared cell edge falls back to the neighbouring cell when the first one has a masked corner.
- [x] `data/gather.cpp`: gridded messages whose `gridList` count differs from the enabled bitmask points are dropped with a warning (prevents silent shifting, e.g. with `--tkr-ignore-bitmask`).
- [x] `data/gather.cpp`: `TOKORO_GRID_DUMP` resolved once instead of `getenv` per residual.
- [x] `diag.cpp`: `mkdir`/`fopen` failures are logged; redundant `mkdir` removed.
- [x] `generator.cpp`: satellite reserve matches the actual list (165).
- [x] Dead code removed: `GridIndex`, `IonosphereGrid*`, `TroposphereGrid*`, `IonosphericCorrection::vtec_*_residual`, `#if 0` blocks in `satellite.cpp`.
- [x] Tests: `tests/generator/tokoro/grid.cpp` (masked points, corners, shared edges, outside grid, count mismatch).

## Improvements

- [ ] `generator.cpp` `find_corrections`: QZSS gnss-ID assistance data is skipped as unsupported, so enabling QZSS gives satellites without SSR corrections.
- [ ] GLONASS generation is unimplemented (`generator.cpp` satellite list, `find_ephemeris`); FDMA channel not modeled in `Observation`.
- [ ] Grid residuals/tropo per point are never cleared when a later gridded message omits a satellite or point; with max ages disabled (client default) a cell can mix epochs indefinitely.
- [ ] Grid is only rebuilt when `correctionPointSetID` changes; a new correction point set with the same ID but different geometry/bitmask keeps the old layout.
- [ ] `decode.cpp` `epoch_time_r15`: SBAS/NavIC time IDs fall back to `Tai::now()`; should reject instead.
- [ ] Masked cell corner: no 3-point or nearest-point fallback, the cell has no correction.
- [ ] `data/grid.cpp`: no handling for grids crossing the ±180° meridian.
- [ ] `observation.cpp` `compute_ranges`: phase-range rate holds STEC, tropo and windup constant between the two states; only geometry and clock contribute.
- [ ] Deduplicate RTCM→ephemeris conversion (`processor/tokoro.cpp`, `processor/rtcm2eph.cpp`, `tokoro-post/main.cpp`) and option validation between `examples/client` and `examples/tokoro-post`.
- [ ] `satellite.cpp` `update()`: diagnostic detail strings are built even when diagnostics are disabled (only on the no-ephemeris path).
- [ ] `data/grid.hpp` `point_from_array_index`: O(n) scan.
- [ ] Dynamic VRS: no diagnostic when location updates go stale.
- [ ] Tests: iono grid requirement, orbit/clock age checks, config CLI parsing.
