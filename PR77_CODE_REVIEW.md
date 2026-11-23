# Code review: PR #77 `fix(tokoro): correct SSR sign, phase wavelength, grid staleness, …`

Branch `t3/fix-tokoro-ssr-sign-phase-wavelength` against `origin/main`, reviewed at `a77b3baf` plus the
uncommitted C++11 fixes listed under "Fixed during this review". About 30 commits and 140 files.

## How this review was done

Five reviewers each read a separate part of the diff in full and wrote a findings report. They looked at:

1. Tokoro `generator.cpp` and diagnostics.
2. Tokoro observation, satellite and `data/*` (SSR grid, interpolation).
3. Tokoro atmosphere and frame models.
4. The RTCM, RINEX, nav, ephemeris, gnss, time and coordinates libraries.
5. The examples, tests, CMake, docker, scripts and docs.

I then checked the highest-impact claims against the code myself. Each finding carries a status:

| Mark | Meaning |
|---|---|
| ✔ | Confirmed by me by reading the code (or running a tool). |
| ◐ | Reported by a reviewer and plausible, but not independently confirmed by me. |
| ✖ | Disputed. Listed separately with the reason. |

**What this review does not cover.** It is a read-through plus the builds and unit tests. It is **not** a
behavioural comparison against `main` on real assistance data. Many changes here are deliberate behaviour
changes (see section 4). Whether they are right for a live SSR network can only be shown by running the
`main` and branch builds on the same capture and diffing the RTCM output.

## 0. Summary

| Severity | Count | Notes |
|---|---|---|
| Critical | 0 | One reviewer-reported Critical (GPT2 pressure) is disputed, see D1. It needs a real grid file to close. |
| High | 4 | Two feature-parity or docs gaps, two library hardening gaps. |
| Medium | 9 | Mostly behaviour or documentation, and two robustness items. |
| Low and nits | 15 | |
| Disputed | 4 | Not issues, or not as stated. |

There is no CI on this branch. Nobody other than the automated reviewers has read the diff.

## 1. High

### H1. `tokoro-post` has no `--ssr-gcc` or `--negative-correction-sign` ✔
- **Where:** `examples/tokoro-post/main.cpp` (no flag, no setter call). Compare
  `examples/client/config/tokoro.cpp:258,286` and `examples/client/processor/tokoro.cpp:66,431`.
- **Problem:** `example-client` exposes both options and wires them to the generator.
  `tokoro-post`, which mirrors almost every other Tokoro option, cannot set them. The two front-ends
  silently diverge. SSR sign problems cannot be reproduced offline with the same settings.
- **Impact:** Capability gap only. Both default to off, so existing output is unchanged.
- **Fix:** Add `--ssr-gcc` and `--negative-correction-sign` to `tokoro-post`. Call
  `generator->set_ssr_generation_consistency_check(...)` and `ref_station->set_negative_correction_sign(...)`.

### H2. README lists only 11 of about 27 new `--tkr-*` flags ✔
- **Where:** `examples/client/README.md` against `examples/client/config/tokoro.cpp`.
- **Missing (spot-checked: none of these appear in the README):** `--tkr-lock-start/-rise`,
  `--tkr-ambiguity-randomize/-seed`, `--tkr-ssr-gcc`, `--tkr-negative-correction-sign`,
  `--tkr-tropo-model/-mapping/-gradient/-gradient-mapping/-gradient-north/-east`, `--tkr-diag-tropo-all`,
  `--tkr-gpt2-grid`, `--tkr-vmf3-grid`, `--tkr-vmf3-orography`.
- **Fix:** Copy the Tokoro section from `example-client --help` into the README. While there, add `grid`
  to the `--tkr-vrs-mode` choices (L13).

### H3. RTCM `MessageFilter` has no `qzss` switch ✔
- **Where:** `dependency/generator/rtcm/include/generator/rtcm/generator.hpp:15-22` and the purge block in
  `generator.cpp`.
- **Problem:** `MessageFilter::systems` has `gps/glonass/galileo/beidou` only. QZSS observations are now
  produced (`GenericGnssId::QZSS`, MT111x), but the filter never removes them. A consumer cannot disable
  QZSS output.
- **Fix:** Add `bool qzss = true;` and erase `GenericGnssId::QZSS` like the other systems. Check callers
  that build a `MessageFilter` (the `lpp2rtcm` and `tokoro` configs).

### H4. `BitReader` has no bounds check ✔ (mitigated by callers today)
- **Where:** `dependency/format/rtcm/messages/bits.hpp:15-24`.
- **Problem:** `u()/s()/sm()` index `mData[mBit / 8]` unchecked. The 1020 and 1044 parsers pre-check the
  total length, so it is safe today. This runs on network input, so a future caller or a wrong bit-count
  constant becomes an out-of-bounds read.
- **Fix:** Check `mBit / 8 >= mData.size()` inside `u()` and set a sticky error flag (or return 0). Have
  `parse()` return failure if the flag is set.

## 2. Medium

### M1. `Rtcm2Eph` resolves the GLONASS day with the wall clock ✔
- **Where:** `examples/client/processor/rtcm2eph.cpp:59`, `format::rtcm::to_ephemeris(*rtcm, ts::Tai::now())`.
- **Problem:** RTCM 1020 carries only the time within the day. For recorded, replayed or delayed streams,
  `now()` can pick the wrong day around midnight. `tokoro-post` passes the message time instead.
- **Fix:** Pass the reception or stream time through `Rtcm2Eph::inspect`.

### M2. RTCM 1020 → GLONASS health ignores `ln3/ln5` ◐
- **Where:** `dependency/format/rtcm/messages/ephemeris.cpp:~179` against `dependency/format/nav/glo/glo.cpp:~114`.
- **Problem:** The SFRBX path uses `((bn >> 2) & 1) | ln`. The RTCM path uses only `bn_msb`. The same
  satellite can read healthy from RTCM and unhealthy from SFRBX.
- **Fix:** OR in `ln3` (and `ln5`) in the RTCM converter.

### M3. MSM cell-count guard is an `assert` only ✔
- **Where:** `dependency/generator/rtcm/messages/msm.cpp:396`, then the read loop at `:409`.
- **Problem:** With `NDEBUG` the guard disappears. If `cell_count` ever exceeds 64, the loop reads past the
  64-element `cell_data`. The partitioning caller prevents it today.
- **Fix:** Clamp or return an error as well as asserting.

### M4. List-of-correction-points path allocates on every call ✔
- **Where:** `dependency/generator/tokoro/data/grid.cpp`, `select_list` and `list_plan` (several
  `std::vector`s per call).
- **Problem:** It runs once per satellite, signal and epoch, but only for list-form sets. Cost is small for
  at most 64 points, yet it sits on the observation path.
- **Fix:** Use fixed-size local arrays (the list is at most 64 points) or reusable scratch buffers.

### M5. Missing-as-zero for lists does not renormalise the weights (design note) ✔
- **Where:** `grid.cpp`, the `Weighted` branch of `interpolate_cell`.
- **Observation:** A point without data keeps its IDW or barycentric weight and contributes 0 (iono) or the
  nominal delay (tropo), so the result is pulled towards 0 or nominal. This is what "missing residual = 0"
  means, and it matches the array path. It is not a bug, but a reviewer read it as one.
- **Fix:** Document it in the `--tkr-grid-missing-as-zero` help and README. If renormalising is wanted
  instead, add it as a separate option.

### M6. VMF3 height reduction has no domain guard ◐
- **Where:** `dependency/generator/tokoro/models/vmf3.cpp:270-274`.
- **Problem:** The sibling models reject heights outside −1000..20000 m. `pow(1 − k·(h − orography), 5.225)`
  becomes NaN for bad orography data. Not reachable with sane inputs.
- **Fix:** Add the same height-domain check.

### M7. Frame connectivity is probed at a fixed epoch 2020.0 ◐
- **Where:** `dependency/generator/tokoro/models/frame_transform.cpp:30,163-165`.
- **Problem:** `OrbitFrames::connected` has no fallback to the real observation epoch. It only runs at setup
  to validate user input, and the graph edges here are epoch-independent today, so impact is low.
- **Fix:** Thread the epoch through, or add a comment on why 2020.0 is sufficient.

### M8. GLONASS dispersive residual scale uses channel k = 0 ◐
- **Where:** `dependency/generator/rtcm/extract/residuals.cpp:57-83`.
- **Problem:** The error is under about 0.6 % and acknowledged in a comment. The helper name suggests more.
- **Fix:** Document it at the call site, or pass the satellite's channel.

### M9. Snapshots do not carry `ssr_iod` ◐
- **Where:** `include/generator/tokoro/snapshot.hpp`, `data/gather.cpp`.
- **Problem:** A snapshot-loaded satellite has `ssr_iod == 0` on both sides, so the SSR-generation check
  can never fire in snapshot tests. This is not documented, unlike the `negative_correction_sign` exclusion.
- **Fix:** Add the field, or note the gap in `TODO.md`.

## 3. Low and nits

| # | Item | Where |
|---|---|---|
| L1 ✔ | `GridData` members (`is_list`, reference lat/lon, steps, bitmask) have no default initializers. Currently safe because `mGrid[gnss]` value-initialises, but cheap hygiene. | `data/grid.hpp` |
| L2 ◐ | `GridData::matches` compares list positions with exact `!=`. Fine today (same decode path), fragile later. | `data/grid.hpp` |
| L3 ◐ | GLONASS residual silently skipped when the frequency channel is unknown. Add a `VERBOSEF`. | `generator.cpp` (`build_rtcm_residual`) |
| L4 ◐ | `diag_discard_detail` passed as `std::string` by value on every signal. | `generator.hpp`, `generator.cpp` |
| L5 ◐ | Antenna-descriptor strings are truncated silently when the 1023-byte payload is exceeded. | `dependency/generator/rtcm/messages/antenna_descriptor.cpp` |
| L6 ◐ | Quantize helpers assume one reserved-pattern convention for every DF. Confirm per call site. | `dependency/generator/rtcm/messages/helper.hpp` |
| L7 ◐ | PZ-90→PZ-90.11 uses 2010.0 as the reference epoch although "no epoch". Inert (zero rates). | `coordinates/transform/pz90.hpp` |
| L8 ◐ | `frame_epoch()` falls back to `{TimeDependent, 0.0}` after an exhaustive switch, hiding `-Wswitch`. | `dependency/coordinates/src/frame.cpp` |
| L9 ◐ | `tokoro-post` `FILE*` is closed only at the end of `main`. A future early return would leak it. | `examples/tokoro-post/main.cpp` |
| L10 ◐ | VMF3 latitude clamp at the poles is silent (GPT2 logs it). | `models/vmf3.cpp` |
| L11 ✔ | PZ-90 flattening changed from 1/298.25 to 1/298.257839303. Looks right (matches PZ-90.11), but confirm against the ICD edition you cite. | `tokoro/coordinates/reference_ellipsoid.cpp` |
| L12 ◐ | `Satellite::disable()` zero-argument overload is dead code. | `satellite.hpp` |
| L13 ✔ | README `--tkr-vrs-mode` still lists `fixed, dynamic` and not `grid` (pre-existing). | `examples/client/README.md` |
| L14 ◐ | Niell height constants duplicated in four files. `mops.cpp` has a stale TODO about `dmin_s`. `TroposphericDelay::mapping_valid` is undocumented. | `tokoro/models/*`, `observation.hpp` |
| L15 ◐ | `Ephemeris::iode()/iodc()` both return `lpp_iod` for GLONASS. Add a one-line comment. | `dependency/ephemeris/include/ephemeris/ephemeris.hpp` |

Informational: `random_phase_ambiguity` has negligible modulo bias (about 1e-16). The 64-bit `bitmaskOfGrids`
cap in `set.cpp` is correct per the ASN.1 but uncommented.

## 4. Behaviour changes to call out in the PR description

These are intentional or likely intentional. They change output or defaults for existing users, so they
belong in the release notes and need real-data validation.

- **Satellite dropped on degenerate orbit correction.** A failed unit-vector normalisation in the orbit
  correction now disables the satellite (`position_failed`) instead of continuing with the uncorrected
  position (`satellite.cpp`).
- **New discard rules:** stale iono, tropo, bias, clock and orbit data (`max_*_age`), SSR-generation
  consistency (`--tkr-ssr-gcc`), per-signal bias freshness, and an incomplete or masked grid cell
  (`four-point` default). These can reduce the number of satellites in the output.
- **SSR decoding fixes:** swapped latitude/longitude step resolution, `dot_delta_*` resolution constants,
  the STEC quality-indicator table direction, and unsupported time systems now drop the message instead of
  using `Tai::now()`.
- **`SignalId::wavelength()`** now returns metres (it was off by 1000). Nothing outside tests called it.
- **`Utc::day_of_year()`** is now one-based. The three callers (`helper.cpp:76`, `mops.cpp:53`,
  `vmf3.cpp:334`) were updated and are consistent with the seasonal models (`(doy − 28)/365.25`, `dmin`).
- **GPS ephemeris validity** now uses the fit interval table (IS-GPS-200N Table 20-XII) with a ±half-interval
  window. Week-rollover handling now also accepts an ephemeris one week behind.
- **GLONASS and QZSS** ephemeris conversion in `lpp2eph/rtcm2eph/ubx2eph` is on by default (opt out with
  `--*-no-glo` / `--*-no-qzs`). Downstream consumers of those tags will see new messages.
- **Idokeido module removed** (`INCLUDE_GENERATOR_IDOKEIDO`, `--idokeido`, `ido` alias and tag, docker
  matrices). The repo has no leftover references except `CHANGELOG.md` history.
- **MSM version selection** now treats resolution and extended-info as independent axes instead of a numeric
  max. This affects all users of the RTCM generator.

## 5. Disputed or not issues

### D1. GPT2 pressure "100× too small" (reported Critical) ✖ (likely false positive, needs a grid file)
- **Claim:** `gpt2.cpp` reads pressure with no unit conversion and then divides by 100, giving 0.023 m
  instead of 2.3 m ZHD.
- **Why I doubt it:** The GPT2 `.grd` file stores pressure in **Pa**. The original `gpt2_5.m` ends with
  `p = p/100` to return hPa. The struct comment says `[Pa]`, and `Gpt2Values::pressure` and
  `saastamoinen_delay` use hPa, which is consistent with `/100` at the end.
- **Action:** Not verifiable here, because there is no `.grd` file on this machine and no GPT2 test. Run
  `--tkr-tropo-model gpt2` with a real grid once and check that the zenith hydrostatic delay is about 2.3 m
  (or add a one-node unit test).

### D2. "ITRF2020→ITRF93 rate sign flipped" ✖
- The changed value is the X-translation **rate** (`dtx`) in `tokoro/coordinates/coordinate.cpp:110`, from
  `2.8` to `-2.8`. The reviewer called it the Y rate.
- The EUREF TN1 table quoted in `dependency/coordinates/include/coordinates/transform/itrf.hpp` lists ITRF93
  rates as `-2.8 -0.2 -2.3 …`, so the new value matches the repo's own reference. Not an issue.

### D3. "`--tokoro` stops consuming ephemeris" (reported High) ✖
- `ubx2eph`, `rtcm2eph` and `lpp2eph` default to `enabled = true`. On `origin/main`, `setup_tokoro` already
  registered only the `TokoroEphemerisGps/Gal/Bds` consumers of those processors. The removed
  `TokoroEphemerisUbx/Rtcm` classes were not registered, so they were dead code. Existing configs still
  get ephemeris.
- Still worth one line in the README that Tokoro depends on the `*2eph` processors.

### D4. "`is_list` can be read uninitialised" (reported Medium) ✖ as stated
- `mGrid[gnss]` value-initialises `GridData`, so the fields are zero. Add in-class initialisers anyway (L1).

## 6. Checked and found OK

- Orbit-correction and STEC signs, the GLONASS FDMA frequency formula, and the STEC quality table.
- The antimeridian wrap, `plane_plan` geometry and `cell_candidates` edge handling in `grid.cpp`.
- Barycentric degenerate-triangle rejection.
- IERS 2010 solid-tide step 1 and step 2.
- VMF3, GMF and Vienna mapping formulas, and the VMF3 coefficient table sizes (91 rows each).
- GPS fit-interval table and week-rollover logic.
- `SignalId::wavelength()` fix, plus the new 1020 and 1044 parsers' length handling.
- PZ-90.11 Helmert sign handling (checked against PROJ `cct`).
- Idokeido removal (CMake, docker, scripts, docs).
- CMake gating for `INCLUDE_GENERATOR_TOKORO`.
- `args.hxx` default-value flags (no always-true bug).
- The new tests are meaningful, not tautological.

## 7. Fixed during this review

- **gcc 4.8 and clang 10 builds** (C++11) now pass on the whole tree, Debug and Release, with and without
  unity build.
  - Structured bindings replaced in `dependency/generator/spartn/generator.cpp`,
    `examples/lpp2spartn/main.cpp` and `examples/client/config.cpp`.
  - `constexpr` removed from the two `std::log`-calling helpers in `dependency/lpp/include/lpp/location_information.hpp`.
  - These files are not part of this PR's diff, so the failures were already on `main`.
- **`TropoDiag*` structs** (`diag.hpp`) lose their default member initialisers so they stay aggregates in C++11.

## 8. Suggested next steps

1. Fix H1 to H4 (small).
2. Close D1 with a real GPT2 grid.
3. A/B run `main` against the branch on a real capture (satellite counts, discard reasons, RTCM diff).
4. Rewrite the PR title and description using section 4.
5. Consider splitting the PR: GLONASS/QZSS support, troposphere models and Idokeido removal are independent.
