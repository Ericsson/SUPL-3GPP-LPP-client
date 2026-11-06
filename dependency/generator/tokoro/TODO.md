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

## Fixed (round 4)

- [x] `decode.cpp` `epoch_time_r15`: returns false for unsupported time IDs (SBAS, NavIC); the message is dropped with a warning instead of being stamped with `Tai::now()`.
- [x] `data/gather.cpp`: grid is rebuilt when the correction point set layout (reference point, steps, bitmask) changes, not only the ID.
- [x] `data/gather.cpp`: each gridded message replaces the previous one; troposphere values and STEC residuals that are no longer provided are cleared (only for the components the message carries).
- [x] `data/grid.cpp`: grids crossing the ±180° meridian; interpolation fractions come from the cell lookup instead of point positions.
- [x] RTCM 1019/1042/1046 → ephemeris conversion shared in `format/rtcm` (`format::rtcm::to_ephemeris`), used by the client Tokoro processor, `Rtcm2Eph` and `tokoro-post`.
- [x] Tests: `tests/generator/tokoro/correction.cpp` (iono grid requirement and poly-only, satellite dropped from a later grid message, grid rebuild on layout change, orbit/clock age from the SSR epoch); antimeridian grid in `grid.cpp`.

## Fixed (round 5): QZSS and GLONASS

- [x] QZSS: SSR assistance data ingested; MSM 1114-1117 (RTCM Table 3.5-111 signal ids); QZSS signal names/RINEX codes; RINEX obs `J` support.
- [x] GLONASS: satellites generated; ephemeris search with the 7-bit tb IOD; FDMA carrier frequency per channel (`SignalId::frequency(channel)`) for wavelength, iono, windup and APC reference; DF419 channel in MSM5/7; MT1230 (zero biases) and MT1031 residuals; RINEX obs `GLONASS SLOT / FRQ #`.
- [x] Ephemeris sources for both: RTCM 1020/1044, UBX-RXM-SFRBX (GLONASS strings with Hamming check, QZSS LNAV), RINEX nav `R`/`J` records, LPP navigation model (GLONASS needs the channel from GNSS-AuxiliaryInformation).
- [x] `lpp2eph.cpp`: scale factors were decimal (`1e-31`) instead of binary (`2^-31`), and Galileo navToe/navToc and I/NAV af0-af2 exponents were wrong.
- [x] RINEX obs satellite labels used the 0-based `to_string()` (G01 written as G00).
- [x] RTCM `clone()` lost the decoded fields (copy constructor only copied the raw bytes); now re-parses. `Message` copy also kept `mType`.
- [x] Removed the unused `TokoroEphemerisUbx` / `TokoroEphemerisRtcm` (ephemeris comes via Ubx2Eph/Rtcm2Eph).

Defaults NOT changed (left as-is on request): client `generate_glonass` defaults to true (GLONASS now really generated), client QZSS is hard-coded off, tokoro-post GLONASS hard-coded off and QZSS on (`--no-qzs`).

Not verified on real data: GLONASS/QZSS SSR corrections (the recorded datasets carry only GPS/GAL/BDS SSR). UBX GLONASS decoding was verified against the IGS brdc file for the same period.

## Fixed (round 6)

- [x] Lpp2Eph: the week (GPS/GAL/BDS/QZS) and the GLONASS tb day are resolved from the GNSS-ReferenceTime of the assistance data (the latest one seen), falling back to the wall clock only when none was received. The week is chosen so that toe is within half a week of the reference time, which also fixes ephemerides near a week rollover.
- [x] Per-GNSS flags for GLONASS/QZSS in the ephemeris converters: `--u2e-no-glo`, `--u2e-no-qzs`, `--r2e-no-glo`, `--r2e-no-qzs`, `--l2e-no-glo`, `--l2e-no-qzs` (default: converted, as before).
- [x] GLONASS broadcast orbit integration step reduced from 60 s to 30 s, the maximum RTCM 10403.4 (3.5.10.1) allows for SSR consistency.

Investigated, not an issue:
- PZ-90.11 to ITRF transformation: SSR orbit corrections are defined relative to the broadcast orbit and give the corrected orbit in the SSR datum (ITRS-related, RTCM 10403.4 3.5.10.1.1), so the frame offset (mm level) is already absorbed by the correction. Transforming the broadcast orbit first would apply it twice.

## Fixed (round 7)

- [x] Option to transform the broadcast orbits of all GNSS to a common frame before the SSR orbit correction: `--tkr-orbit-frame <frame>` / `--orbit-frame` (default off), with per-GNSS overrides `--tkr-orbit-source-frame <gnss>=<frame>` / `--orbit-source-frame`. Defaults: GPS WGS 84 by GPS week, GLONASS PZ-90.11; Galileo/BeiDou/QZSS only when a source frame is given. New PZ-90.11 <-> ITRF2008 edge (epoch 2010.0, no rates; values and rotation sign convention to be checked against the PZ-90.11 reference document). `frame_from_gps_week` now returns WGS 84 (G2296) from week 2296.
- [ ] Coordinates library: no GTRF (Galileo), JGS (QZSS) or CGCS2000 (BeiDou) transformations; WGS 84 only G2296 is connected (older realizations are not transformed).

## Fixed (round 8)

- [x] `satelliteReferenceDatum-r15` is read; a regional datum is warned about once per GNSS (the datum itself is not in the message).
- [x] VRS position frame: `--tkr-fixed-frame`, `--tkr-fixed-epoch`, `--tkr-fixed-velocity`, `--tkr-ssr-frame` (default itrf2020) in the client fixed VRS mode; `--pos-frame`, `--pos-epoch`, `--pos-velocity`, `--ssr-frame` in tokoro-post. The configured position is transformed to the SSR frame at the observation epoch (recomputed daily), using the coordinates transform graph (SWEREF 99 via NKG_RF17vel, ETRF/ITRF via Helmert) and the station velocity for epoch changes. The RTCM 1005/1006 position stays as configured, so a rover gets its position in the configured datum.

## Fixed (round 9): GLONASS/QZSS

- [x] GLONASS ephemeris selection: prefer the ephemeris whose tb matches the SSR IOD (even with the IOD consistency check off), otherwise the one with tb closest to the time. Previously the earliest valid one was used, i.e. up to 30 min of extrapolation (2-3 m orbit error vs about 1 m at 15 min).
- [x] Opt-in switches, defaults unchanged: `--tkr-qzss` (client, QZSS was hard-coded off) and `--glo` (tokoro-post, GLONASS was hard-coded off).

Verification (broadcast-only, zero SSR corrections, dataset2 22:50-23:50, against the Trimble receiver at the VRS, `ntrip-vrs.obs`, Tokoro RTCM MSM7 converted with convbin):
- Frequency channels from DF419 match the receiver for all common satellites.
- Iono-free code minus Tokoro code (per-epoch median removed): GPS std 0.05-0.11 m, GLONASS std 0.2-0.6 m with per-satellite offsets up to 6.6 m (receiver GLONASS code inter-frequency biases and broadcast clock errors); 10 s phase-rate residual GPS 3-16 mm, GLONASS 15-55 mm (outliers at ephemeris switches).
- QZSS (synthetic VRS in Tokyo): MSM 1114-1117 decode in convbin as C1C/L1C, C2X/L2X, C5X/L5X with consistent code/phase.
- Not verified: GLONASS/QZSS SSR corrections (no recording with GLONASS/QZSS SSR).

## Verified with GLONASS SSR (round 10)

Session `tokoro_compare/sessions/itrf-vrs_20261022_140701` (6 h, LPP SSR with GLONASS, device RTCM, provider VRS `SE_Lin_Office_ITRF`), `tokoro-post --glo` at the Lin Office VRS, compared with the provider's VRS observations (per-epoch median removed, phase per arc):

| | code std | phase std |
|---|---|---|
| GPS L1C / L2W | 6 / 8 mm | 4.6 / 5.9 mm |
| GLONASS G1C / G2C (13 sats) | 6 / 8 mm | 5.5 / 6.3 mm |
| Galileo E1 / E6 | 6 / 7 mm | 9.8 / 4.7 mm |
| BeiDou B1I / B3I | 9 / 11 mm | 4.4 / 5.8 mm |

- `--orbit-frame itrf2020` changes GLONASS ranges by 4 +- 2 mm and fits the provider slightly worse (G1C phase 5.9 vs 5.5 mm): weak evidence that the corrections refer to the untransformed broadcast orbit.
- The phase arcs break at jumps common to all satellites of a system (e.g. -1.44 / -6.2 cycles on GPS L1); these are the known reference-side events, not GLONASS specific.

Fixed while doing this:
- [x] RINEX nav reader: RINEX 4 (`> EPH <sat> <type>` records, also STO/EOP/ION skipped); before, a RINEX 4 file lost sync and loaded almost nothing.
- [x] Diag file names for GLONASS/QZSS signals were `??.diag`.

Found, not fixed (data source): the receiver in the session sends GLONASS RTCM 1020 that are days old (one fixed tb per slot for the whole session, NT = 1009-1019 while the day is 1026), so GLONASS needs `--nav` with a broadcast file there.

## Improvements

- [ ] Station velocity models are compiled in (NKG_RF17vel, Nordic only); no external velocity grid files.
- [ ] Shared broadcast-orbit frame handling for Idokeido SPP (broadcast-only orbits).


- [ ] Masked cell corner: no 3-point or nearest-point fallback, the cell has no correction.
- [ ] `observation.cpp` `compute_ranges`: phase-range rate holds STEC, tropo and windup constant between the two states; only geometry and clock contribute.
- [ ] `satellite.cpp` `update()`: diagnostic detail strings are built even when diagnostics are disabled (only on the no-ephemeris path).
- [ ] `data/grid.hpp` `point_from_array_index`: O(n) scan.
- [ ] Dynamic VRS: no diagnostic when location updates go stale (needs a staleness limit to be decided).
- [ ] Tests: config CLI parsing (no test harness for `examples/client` yet).
