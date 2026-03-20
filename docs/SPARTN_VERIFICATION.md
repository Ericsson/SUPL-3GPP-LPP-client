# SPARTN Field Verification Status

Verification against SPARTN v2.0.1 specification (`build/210928_SPARTN_v2.0.1.pdf`)

## Verified Correct ✓

| Field | Name | Status | Notes |
|-------|------|--------|-------|
| SF005 | Solution Issue of Update (SIOU) | ✓ | 9 bits |
| SF011 | GPS Satellite Mask | ✓ | 32/44/56/64 bits |
| SF012 | GLONASS Satellite Mask | ✓ | 24/36/48/63 bits |
| SF013 | Do Not Use (DNU) | ✓ | 1 bit |
| SF014 | OCB Present Flags | ✓ | 3 bits |
| SF015 | Continuity Indicator | ✓ | 3 bits |
| SF018 | GPS IODE | ✓ | 8 bits |
| SF019 | GLONASS IODE | ✓ | 7 bits |
| SF020 | Satellite Corrections | ✓ | 14 bits, ±16.382m, 0.002m |
| SF021 | Satellite Yaw | ✓ | 6 bits, 0-354°, 6° |
| SF024 | User Range Error (URE) | ✓ | 3 bits |
| SF025 | GPS Phase Bias Mask | ✓ | 7 or 12 bits |
| SF026 | GLONASS Phase Bias Mask | ✓ | 6 or 10 bits |
| SF027 | GPS Code Bias Mask | ✓ | 7 or 12 bits |
| SF028 | GLONASS Code Bias Mask | ✓ | 6 or 10 bits |
| SF029 | Code Bias Correction | ✓ | 11 bits, ±20.46m, 0.02m |
| SF030 | Area Count | ✓ | 5 bits |
| SF031 | Area ID | ✓ | 8 bits |
| SF032 | Area Reference Latitude | ✓ | 11 bits |
| SF033 | Area Reference Longitude | ✓ | 12 bits |
| SF034 | Area Latitude Grid Node Count | ✓ | 3 bits |
| SF035 | Area Longitude Grid Node Count | ✓ | 3 bits |
| SF036 | Area Latitude Grid Node Spacing | ✓ | 5 bits |
| SF037 | Area Longitude Grid Node Spacing | ✓ | 5 bits |
| SF039 | Number of Grid Points Present | ✓ | 7 bits |
| SF040 | Poly/Grid Block Present Indicator | ✓ | 2 bits |
| SF041 | Troposphere Equation Type | ✓ | 3 bits |
| SF042 | Troposphere Quality | ✓ | 3 bits |
| SF043 | Area Average Vertical Hydrostatic Delay | ✓ | |
| SF044 | Troposphere Polynomial Coeff Size Indicator | ✓ | 1 bit |
| SF045 | Small Troposphere Coefficient T00 | ✓ | 7 bits, ±0.252m, 0.004m |
| SF046 | Troposphere Coefficient T10/T01 | ✓ | 7 bits |
| SF047 | Small Troposphere Coefficient T11 | ✓ | 9 bits |
| SF048 | Large Troposphere Coefficient T00 | ✓ | 9 bits, ±1.020m, 0.004m |
| SF049 | Large Troposphere Coefficient T10/T01 | ✓ | 9 bits |
| SF050 | Large Troposphere Coefficient T11 | ✓ | 11 bits |
| SF051 | Troposphere Residual Field Size | ✓ | 1 bit |
| SF052 | Small Troposphere Residual | ✓ | 6 bits |
| SF053 | Large Troposphere Residual | ✓ | 8 bits |
| SF054 | Ionosphere Equation Type | ✓ | 3 bits |
| SF055 | Ionosphere Quality | ✓ | 4 bits |
| SF056 | Ionosphere Polynomial Coeff Size Indicator | ✓ | 1 bit |
| SF057 | Small Ionosphere Coefficient C00 | ✓ | 12 bits, ±81.88 TECU, 0.04 |
| SF058 | Small Ionosphere Coefficient C10/C01 | ✓ | 12 bits |
| SF059 | Small Ionosphere Coefficient C11 | ✓ | 13 bits |
| SF060 | Large Ionosphere Coefficient C00 | ✓ | 14 bits |
| SF061 | Large Ionosphere Coefficient C10/C01 | ✓ | 14 bits |
| SF062 | Large Ionosphere Coefficient C11 | ✓ | 15 bits |
| SF063 | Ionosphere Residual Field Size | ✓ | 2 bits |
| SF064 | Small Ionosphere Residual | ✓ | 4 bits, ±0.28 TECU |
| SF065 | Medium Ionosphere Residual | ✓ | 7 bits, ±2.52 TECU |
| SF066 | Large Ionosphere Residual | ✓ | 10 bits, ±20.44 TECU |
| SF067 | Extra-Large Ionosphere Residual | ✓ | 14 bits, ±327.64 TECU |
| SF068 | Area Issue of Update (AIOU) | ✓ | 4 bits |
| SF069 | Reserved | ✓ | 1 bit |
| SF093 | Galileo Satellite Mask | ✓ | 36/45/54/64 bits |
| SF094 | BeiDou Satellite Mask | ✓ | 37/46/55/64 bits (FIXED) |
| SF099 | Galileo IODnav | ✓ | 10 bits |
| SF100 | BeiDou IODE/IODC | ✓ | 8 bits (FIXED) |
| SF102 | Galileo Phase Bias Mask | ✓ | 9 or 16 bits |
| SF103 | BDS Phase Bias Mask | ✓ | 9 or 16 bits |
| SF104 | QZSS Phase Bias Mask | ✓ | 7 or 12 bits |
| SF105 | Galileo Code Bias Mask | ✓ | 9 or 16 bits |
| SF106 | BDS Code Bias Mask | ✓ | 9 or 16 bits |
| SF107 | QZSS Code Bias Mask | ✓ | 7 or 12 bits |

## Bugs Fixed

| Field | Issue | Fix |
|-------|-------|-----|
| SF094 | BeiDou mask sizes were 10/40/48/64 | Changed to 37/46/55/64 |
| SF100 | BeiDou IODE was 10 bits | Changed to 8 bits |
| satellite_mask() | Used count instead of highest PRN | Now tracks highest PRN |
| sfxxx_bias_mask() | Used count instead of highest type | Now uses `rbegin()->first` |
| orbit_iode() | iode_shift parameter incorrectly defaulted to true | Removed parameter entirely |
| BDS time | 14 seconds off (TAI-33 vs TAI-19) | Use time library for proper conversion |
| GLO time | ~3 hours off (ignored UTC+3 + leap seconds) | Use time library for proper conversion |

## Not Checked Yet

| Field | Name | Notes |
|-------|------|-------|
| SF001 | Message Type | Transport layer |
| SF002 | Message Subtype | Transport layer |
| SF003 | Time Tag Type | Transport layer |
| SF006 | Solution ID | |
| SF007 | Solution Processor ID | |
| SF008 | Encryption Flag | |
| SF009 | Satellite Reference Datum | |
| SF010 | End of OCB Set | |
| SF016 | GPS Ephemeris Type | |
| SF017 | GLONASS Ephemeris Type | |
| SF022 | IODE Continuity | |
| SF023 | Fix Flag | |
| SF038 | Grid Point Bitmask | |
| SF070-SF080 | Various reserved/auth fields | |
| SF081 | VTEC Size Indicator | BPAC message |
| SF082 | Small VTEC Residual | BPAC message |
| SF083 | Large VTEC Residual | BPAC message |
| SF095 | QZSS Satellite Mask | QZSS disabled by default |
| SF096 | Galileo Ephemeris Type | |
| SF097 | BDS Ephemeris Type | |
| SF098 | QZSS Ephemeris Type | |
| SF101 | QZSS IODE | QZSS disabled by default |

## Not Implemented (By Design)

| Field | Name | Reason |
|-------|------|--------|
| SF095 | QZSS Satellite Mask | QZSS support disabled by default |
| SF101 | QZSS IODE | QZSS support disabled by default |
| SM 3-0 | BPAC Message | Basic precision not implemented |
| SM 4-0 | Dynamic Key Message | Encryption not implemented |
| SM 4-1 | Group Auth Message | Deprecated |

## Time Handling

- ✓ Reference epoch: January 1, 2010, 00:00:00 GPS time
- ✓ GPS time conversion (verified identical to previous implementation)
- ✓ Galileo time conversion (verified identical to previous implementation)
- ✓ BeiDou time conversion (FIXED - was 14 seconds off due to TAI offset difference)
- ✓ GLONASS time conversion (FIXED - was ~3 hours off due to UTC+3 and leap seconds)

## Signal Type Mappings

- ✓ GPS phase bias: L1C, L2W, L2L, L5Q
- ✓ GPS code bias: C1C, C2W, C2L, C5Q
- ✓ GLONASS phase bias: L1C, L2C
- ✓ GLONASS code bias: C1C, C2C
- ✓ Galileo phase bias: L1C, L5Q, L7Q
- ✓ Galileo code bias: C1C, C5Q, C7Q
- ✓ BDS phase bias: L2I, L5P, L7I, L6I
- ✓ BDS code bias: C2I, C5P, C7I, C6I

---
Last updated: 2025-02-03
