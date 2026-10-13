#!/usr/bin/env python3
"""Analyze BDS IOD compatibility between LPP and SPARTN."""

# BDS ICD: toe_raw is 17 bits, scale factor 8
# toe_sec = toe_raw * 8
# SPARTN SF100: mod(toe_sec / 720, 240)  [using integer division]
# LPP iod_r15: "11 MSB bits of toe"

# Two interpretations of LPP iod:
#   A) iod = msb(toe_raw, 11) = toe_raw >> 6
#   B) iod = msb(toe_sec, 11) = toe_sec >> 9

# For SPARTN conversion from iod, two ways to recover toe:
#   From A: toe_sec_recovered = iod * 64 * 8 = iod * 512
#   From B: toe_sec_recovered = iod * 512
# (same thing, as we discussed)

# But what if someone interprets "11 MSB of toe" differently?
#   C) iod = toe_raw >> 6  (11 from 17-bit raw)
#      and then SPARTN uses: mod(iod * 512 / 720, 240)
#   D) iod = toe_sec >> 9  (11 from 20-bit sec)
#      and then SPARTN uses: mod(iod * 512 / 720, 240)
# C and D are identical as shown.

# The real test: does mod(iod * 512 / 720, 240) == mod(toe_sec / 720, 240)?

errors_hourly = 0
errors_all = 0
errors_all_64 = 0
total_hourly = 0
total_all = 0

max_toe_raw = 604800 // 8  # max valid toe_raw within a BDS week

print("=== Testing all valid toe_raw values (0 to {}) ===".format(max_toe_raw))
print()

# Collect error examples
error_examples = []

for toe_raw in range(max_toe_raw + 1):
    toe_sec = toe_raw * 8
    total_all += 1

    # True SPARTN SF100 (from toe_sec)
    sf100_true = (toe_sec // 720) % 240

    # True SPARTN SF100 if computed from toe_raw directly
    sf100_true_raw = (toe_raw // 720) % 240

    # LPP iod (both interpretations give same value)
    iod = toe_raw >> 6  # = toe_sec >> 9

    # SPARTN from LPP iod: interpretation 1 (iod * 512 / 720) — iod represents toe_sec/512
    sf100_from_iod_512 = (iod * 512 // 720) % 240

    # SPARTN from LPP iod: interpretation 2 (iod * 64 / 720) — iod represents toe_raw/64
    sf100_from_iod_64 = (iod * 64 // 720) % 240

    if sf100_true != sf100_from_iod_512:
        errors_all += 1
    if sf100_true_raw != sf100_from_iod_64:
        errors_all_64 += 1
    if len(error_examples) < 10 and (sf100_true != sf100_from_iod_512 or sf100_true_raw != sf100_from_iod_64):
        error_examples.append((toe_raw, toe_sec, iod, sf100_true, sf100_from_iod_512, sf100_true_raw, sf100_from_iod_64))

print("All toe_raw values (step=1, multiples of 8 seconds):")
print(f"  Total: {total_all}")
print(f"  Errors (iod*512/720%240 vs toe_sec/720%240):  {errors_all} ({100*errors_all/total_all:.2f}%)")
print(f"  Errors (iod*64/720%240  vs toe_raw/720%240):  {errors_all_64} ({100*errors_all_64/total_all:.2f}%)")
print()

# Test only hourly boundaries (normal operation)
print("=== Hourly toe values (multiples of 3600 seconds) ===")
error_examples_hourly = []
errors_hourly_512 = 0
errors_hourly_64 = 0
for hour in range(168):  # 168 hours in a week
    toe_sec = hour * 3600
    toe_raw = toe_sec // 8
    total_hourly += 1

    sf100_true = (toe_sec // 720) % 240
    sf100_true_raw = (toe_raw // 720) % 240
    iod = toe_raw >> 6
    sf100_512 = (iod * 512 // 720) % 240
    sf100_64 = (iod * 64 // 720) % 240

    if sf100_true != sf100_512:
        errors_hourly_512 += 1
    if sf100_true_raw != sf100_64:
        errors_hourly_64 += 1
    error_examples_hourly.append((toe_raw, toe_sec, iod, sf100_true, sf100_512, sf100_true_raw, sf100_64))

print(f"  Total: {total_hourly}")
print(f"  Errors (iod*512/720%240 vs toe_sec/720%240):  {errors_hourly_512} ({100*errors_hourly_512/total_hourly:.2f}%)")
print(f"  Errors (iod*64/720%240  vs toe_raw/720%240):  {errors_hourly_64} ({100*errors_hourly_64/total_hourly:.2f}%)")
print()

if error_examples_hourly:
    print("  Hourly examples:")
    print(f"  {'toe_raw':>8} {'toe_sec':>8} {'iod':>5} {'sec/720':>7} {'*512':>5} {'raw/720':>7} {'*64':>5}")
    for row in error_examples_hourly[:15]:
        print(f"  {row[0]:>8} {row[1]:>8} {row[2]:>5} {row[3]:>7} {row[4]:>5} {row[5]:>7} {row[6]:>5}")
    print()

# Show some error examples from all values
if error_examples:
    print("=== First 10 error examples (all toe values) ===")
    print(f"{'toe_raw':>8} {'toe_sec':>8} {'iod':>5} {'sec/720':>7} {'*512':>5} {'raw/720':>7} {'*64':>5}")
    for row in error_examples:
        print(f"{row[0]:>8} {row[1]:>8} {row[2]:>5} {row[3]:>7} {row[4]:>5} {row[5]:>7} {row[6]:>5}")
    print()

# Now test: what if iod meant toe_raw directly (not >>6 but some other interpretation)?
# E.g., what if someone thought iod = toe_sec / 512 (rounding differently)?
print("=== Sanity check: is iod always == toe_sec // 512? ===")
mismatches = 0
for toe_raw in range(max_toe_raw + 1):
    toe_sec = toe_raw * 8
    iod_from_raw = toe_raw >> 6
    iod_from_sec = toe_sec >> 9
    if iod_from_raw != iod_from_sec:
        mismatches += 1
print(f"  msb(toe_raw,11) vs msb(toe_sec,11) mismatches: {mismatches}")
print()

# Final: what percentage of toe values that are multiples of 720 have no error?
print("=== toe values that are multiples of 720 seconds ===")
errors_720 = 0
total_720 = 0
for toe_sec in range(0, 604800 + 1, 720):
    if toe_sec % 8 != 0:
        continue  # not a valid BDS toe
    toe_raw = toe_sec // 8
    total_720 += 1
    sf100_true = (toe_sec // 720) % 240
    iod = toe_raw >> 6
    sf100_from_iod = (iod * 512 // 720) % 240
    if sf100_true != sf100_from_iod:
        errors_720 += 1

print(f"  Total: {total_720}")
print(f"  Errors: {errors_720} ({100*errors_720/total_720:.2f}%)")

# Test if adding an offset fixes the rounding
print("=== Testing offsets: (iod*512 + offset) / 720 % 240 vs toe_sec/720%240 ===")
print(f"{'offset':>7} {'errors':>10} {'%':>7}")
max_toe_raw_val = 604800 // 8
for offset in range(0, 512, 8):
    err = 0
    for toe_raw in range(max_toe_raw_val + 1):
        toe_sec = toe_raw * 8
        iod = toe_raw >> 6
        if (toe_sec // 720) % 240 != ((iod * 512 + offset) // 720) % 240:
            err += 1
    print(f"{offset:>7} {err:>10} {100*err/(max_toe_raw_val+1):>6.2f}%")

print()
print("=== Testing offsets: (iod*64 + offset) / 720 % 240 vs toe_raw/720%240 ===")
print(f"{'offset':>7} {'errors':>10} {'%':>7}")
for offset in range(0, 64):
    err = 0
    for toe_raw in range(max_toe_raw_val + 1):
        iod = toe_raw >> 6
        if (toe_raw // 720) % 240 != ((iod * 64 + offset) // 720) % 240:
            err += 1
    print(f"{offset:>7} {err:>10} {100*err/(max_toe_raw_val+1):>6.2f}%")

