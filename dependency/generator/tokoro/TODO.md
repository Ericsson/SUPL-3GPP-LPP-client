# Tokoro TODO

## Bugs

- [x] `data/set.cpp` `array_to_index`: `1ULL << (64 - 1 - i)` is UB when the grid has more than 64 points (`numberOfSteps*` are 0..63 but the bitmask is SIZE(64)). Treat points beyond bit 63 as invalid.
- [x] `data/gather.cpp` gridded init: when `array_to_index` fails, an uninitialized `CorrectionPointInfo` is passed to `add_point`, which indexes `grid_points[absolute_index]` with garbage. Skip the point instead.
- [x] `examples/client/processor/tokoro.cpp` periodic callback: `ASSERT(mSystem)` aborts if a time-step tick fires before the first `inspect()`. Skip the tick until `mSystem` is set.
- [x] `examples/client/config/tokoro.cpp`: help choices missing `grid` (`--tkr-vrs-mode`) and `time-step-last` (`--tkr-gen`).

## Risks

- [ ] Orbit/clock age checks use `epoch + update_interval/2` (`data/gather.cpp:105,166`, `data/correction.cpp:17,36`); biases/STEC use raw epoch. Can hide up to `update_interval/2` of staleness.
- [ ] `data/ionosphere.cpp:129-144`: grid residual failure returns success with zero residual; not visible via `data_status`.
- [ ] `models/troposphere_models.cpp` `height_scaling()`: baseline uses ellipsoidal height 0 rather than geoid/MSL; verify reference surface for SSR gridded tropo.
- [ ] `decode.cpp:30-36`: latitude/longitude step resolution constants are swapped (harmless while both are 0.01).
- [ ] `satellite.cpp:205-233`: emission-time iteration silently accepts non-convergence.
- [ ] `generator.cpp:1148`: GPS/QZSS IOD matched on low 8 bits only. Client (`processor/tokoro.cpp:154`) sets `lpp_iod = iode`, tokoro-post (`main.cpp:461`) uses `iodc`; make them consistent.
- [ ] `observation.hpp:238-240`: `mPhaseRange`, `mPhaseRangeRate`, `mCodeRange` not default-initialized.
- [ ] `satellite.cpp:55-65`: clock lookup depends on orbit lookup having run first (implicit ordering).
- [ ] Only `GNSS-SSR-ArrayOfCorrectionPoints-r16` is supported; list-of-points and r18 `ArrayOfGridPoints` (bog16/64/256) are not.

## Improvements

- [ ] Front-end parity: QZSS hard-coded off in client but configurable in tokoro-post; `--tkr-no-glonass` exists in client but GLONASS generation is unimplemented (`generator.cpp:134-137`).
- [ ] Deduplicate RTCM→ephemeris conversion and option validation between `examples/client` and `examples/tokoro-post`.
- [ ] `diag.cpp`: log when `fopen`/`mkdir` fails; simplify `mkdirs()`.
- [ ] `satellite.cpp:38-88`: per-epoch diagnostic string building even when diagnostics are disabled.
- [ ] `generator.cpp:94`: satellite reserve is 127, actual count is 141.
- [ ] `models/helper.hpp:39-62` `range_time_division`: single-step overflow fix-up; use a loop or assert.
- [ ] Remove dead code: `GridIndex`, `IonosphereGrid*`, `TroposphereGrid*`, `IonosphericCorrection::vtec_*_residual`, `#if 0` blocks in `satellite.cpp`.
- [ ] `data/grid.hpp:122-127` `point_from_array_index`: O(n) scan.
- [ ] GLONASS FDMA channel not modeled in `Observation` (needed before GLONASS support).
- [ ] Dynamic VRS: no diagnostic when location updates go stale.
- [ ] Tests: grid bitmask/`array_to_index`, config CLI parsing.
