#!/usr/bin/env python3
import subprocess
import os
import json
import time
import shutil
import sys
import argparse
from ui import console, Status, status_style, create_progress, format_size
from rich.table import Table
from rich.text import Text

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(os.path.expanduser('~/.cache/SUPL-3GPP-LPP-client'), 'sizes')
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

# Only build example-client; disable all others to avoid spurious failures
ONLY_CLIENT = [
    '-DBUILD_EXAMPLE_NTRIP=OFF',
    '-DBUILD_EXAMPLE_MODEM_CTRL=OFF',
    '-DBUILD_EXAMPLE_CTRL_TOGGLE=OFF',
    '-DBUILD_EXAMPLE_LPP2SPARTN=OFF',
    '-DBUILD_EXAMPLE_TRANSFORM=OFF',
]

CONFIGS = [
    {'name': 'debug',        'flags': ['-DCMAKE_BUILD_TYPE=Debug']},
    {'name': 'baseline',     'flags': ['-DCMAKE_BUILD_TYPE=Release']},
    {'name': 'size-opt',     'flags': ['-DCMAKE_CXX_FLAGS=-Os']},
    {'name': 'gc-sections',  'flags': ['-DCMAKE_CXX_FLAGS=-Os -ffunction-sections -fdata-sections',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-Wl,--gc-sections']},
    {'name': 'lto',          'flags': ['-DCMAKE_CXX_FLAGS=-Os -flto',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-flto']},
    {'name': 'full-opt',     'flags': ['-DCMAKE_CXX_FLAGS=-Os -flto -ffunction-sections -fdata-sections',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-flto -Wl,--gc-sections']},
    {'name': 'no-logging',   'flags': ['-DDISABLE_LOGGING=ON']},
    {'name': 'no-tracing',   'flags': ['-DDISABLE_TRACE=ON']},
    {'name': 'no-spartn-tokoro', 'flags': ['-DINCLUDE_GENERATOR_SPARTN=OFF',
                                            '-DINCLUDE_GENERATOR_TOKORO=OFF']},
    {'name': 'no-rtcm',      'flags': ['-DINCLUDE_GENERATOR_RTCM=OFF',
                                        '-DINCLUDE_GENERATOR_SPARTN=OFF',
                                        '-DINCLUDE_GENERATOR_TOKORO=OFF',
                                        '-DINCLUDE_GENERATOR_IDOKEIDO=OFF',
                                        '-DINCLUDE_FORMAT_RINEX=OFF',
                                        '-DINCLUDE_FORMAT_ANTEX=OFF']},
    {'name': 'iot-optimized','flags': ['-DCMAKE_CXX_FLAGS=-Os -flto -ffunction-sections -fdata-sections',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-flto -Wl,--gc-sections',
                                        '-DDISABLE_LOGGING=ON',
                                        '-DDATA_TRACING=OFF',
                                        '-DINCLUDE_GENERATOR_SPARTN=OFF',
                                        '-DINCLUDE_GENERATOR_TOKORO=OFF']},
    {'name': 'iot-min',      'flags': ['-DCMAKE_CXX_FLAGS=-Os -flto -ffunction-sections -fdata-sections',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-flto -Wl,--gc-sections',
                                        '-DDISABLE_LOGGING=ON',
                                        '-DDATA_TRACING=OFF',
                                        '-DINCLUDE_GENERATOR_RTCM=OFF',
                                        '-DINCLUDE_GENERATOR_SPARTN=OFF',
                                        '-DINCLUDE_GENERATOR_TOKORO=OFF',
                                        '-DINCLUDE_GENERATOR_IDOKEIDO=OFF',
                                        '-DINCLUDE_FORMAT_RINEX=OFF',
                                        '-DINCLUDE_FORMAT_ANTEX=OFF']},
    {'name': 'no-pie',       'flags': ['-DCMAKE_CXX_FLAGS=-Os -ffunction-sections -fdata-sections',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-Wl,--gc-sections -no-pie',
                                        '-DDISABLE_LOGGING=ON',
                                        '-DDATA_TRACING=OFF',
                                        '-DINCLUDE_GENERATOR_RTCM=OFF',
                                        '-DINCLUDE_GENERATOR_SPARTN=OFF',
                                        '-DINCLUDE_GENERATOR_TOKORO=OFF',
                                        '-DINCLUDE_GENERATOR_IDOKEIDO=OFF',
                                        '-DINCLUDE_FORMAT_RINEX=OFF',
                                        '-DINCLUDE_FORMAT_ANTEX=OFF']},
    {'name': 'no-xer-jer',   'flags': ['-DCMAKE_CXX_FLAGS=-Os -ffunction-sections -fdata-sections',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-Wl,--gc-sections -no-pie',
                                        '-DDISABLE_LOGGING=ON',
                                        '-DDATA_TRACING=OFF',
                                        '-DINCLUDE_GENERATOR_RTCM=OFF',
                                        '-DINCLUDE_GENERATOR_SPARTN=OFF',
                                        '-DINCLUDE_GENERATOR_TOKORO=OFF',
                                        '-DINCLUDE_GENERATOR_IDOKEIDO=OFF',
                                        '-DINCLUDE_FORMAT_RINEX=OFF',
                                        '-DINCLUDE_FORMAT_ANTEX=OFF',
                                        '-DASN1_XER_SUPPORT=OFF',
                                        '-DASN1_JER_SUPPORT=OFF']},
    {'name': 'absolute-min', 'flags': ['-DCMAKE_CXX_FLAGS=-Os -flto -ffunction-sections -fdata-sections',
                                        '-DCMAKE_EXE_LINKER_FLAGS=-flto -Wl,--gc-sections -no-pie',
                                        '-DDISABLE_LOGGING=ON',
                                        '-DDISABLE_TRACE=ON',
                                        '-DDATA_TRACING=OFF',
                                        '-DINCLUDE_GENERATOR_RTCM=OFF',
                                        '-DINCLUDE_GENERATOR_SPARTN=OFF',
                                        '-DINCLUDE_GENERATOR_TOKORO=OFF',
                                        '-DINCLUDE_GENERATOR_IDOKEIDO=OFF',
                                        '-DINCLUDE_FORMAT_RINEX=OFF',
                                        '-DINCLUDE_FORMAT_ANTEX=OFF',
                                        '-DASN1_XER_SUPPORT=OFF',
                                        '-DASN1_JER_SUPPORT=OFF']},
]

# Per-option configs: each toggles exactly one flag vs Release baseline
OPTION_CONFIGS = [
    {'option': 'INCLUDE_GENERATOR_RTCM',    'flags': ['-DINCLUDE_GENERATOR_RTCM=OFF',
                                                       '-DINCLUDE_GENERATOR_SPARTN=OFF',
                                                       '-DINCLUDE_GENERATOR_TOKORO=OFF',
                                                       '-DINCLUDE_GENERATOR_IDOKEIDO=OFF',
                                                       '-DINCLUDE_FORMAT_RINEX=OFF',
                                                       '-DINCLUDE_FORMAT_ANTEX=OFF']},
    {'option': 'INCLUDE_GENERATOR_SPARTN',  'flags': ['-DINCLUDE_GENERATOR_SPARTN=OFF',
                                                       '-DINCLUDE_GENERATOR_TOKORO=OFF']},
    {'option': 'INCLUDE_GENERATOR_TOKORO',  'flags': ['-DINCLUDE_GENERATOR_TOKORO=OFF']},
    {'option': 'INCLUDE_GENERATOR_IDOKEIDO','flags': ['-DINCLUDE_GENERATOR_IDOKEIDO=OFF']},
    {'option': 'INCLUDE_FORMAT_RINEX',      'flags': ['-DINCLUDE_FORMAT_RINEX=OFF']},
    {'option': 'INCLUDE_FORMAT_ANTEX',      'flags': ['-DINCLUDE_FORMAT_ANTEX=OFF']},
    {'option': 'DISABLE_LOGGING',           'flags': ['-DDISABLE_LOGGING=ON']},
    {'option': 'DISABLE_TRACE',             'flags': ['-DDISABLE_TRACE=ON']},
    {'option': 'DATA_TRACING',              'flags': ['-DDATA_TRACING=ON']},
    {'option': 'ASN1_XER_SUPPORT',          'flags': ['-DASN1_XER_SUPPORT=OFF', '-DASN1_JER_SUPPORT=OFF']},
    {'option': 'ASN1_JER_SUPPORT',          'flags': ['-DASN1_JER_SUPPORT=OFF']},
    {'option': 'no-pie',                    'flags': ['-DCMAKE_EXE_LINKER_FLAGS=-no-pie']},
]


def get_size(binary):
    result = subprocess.run(['size', binary], capture_output=True, text=True)
    lines = result.stdout.strip().split('\n')
    if len(lines) < 2:
        return None
    parts = lines[1].split()
    return {'text': int(parts[0]), 'data': int(parts[1]), 'bss': int(parts[2]), 'total': int(parts[3])}


def get_file_size(path):
    return os.path.getsize(path) if os.path.exists(path) else 0


def run_bloaty(binary, baseline_binary=None, datasource='symbols'):
    cmd = ['bloaty', '--csv', '-d', datasource, binary]
    if baseline_binary and os.path.exists(baseline_binary):
        cmd += ['--', baseline_binary]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        return None
    rows = []
    for line in result.stdout.strip().split('\n')[1:]:  # skip header
        parts = line.split(',')
        if len(parts) >= 3:
            try:
                rows.append({'name': parts[0], 'vmsize': int(parts[1]), 'filesize': int(parts[2])})
            except ValueError:
                continue
    rows.sort(key=lambda r: abs(r['filesize']), reverse=True)
    return rows[:20]


def build_config(name, extra_flags, build_type='Release'):
    build_dir = os.path.join(BUILD_DIR, name)
    os.makedirs(build_dir, exist_ok=True)

    all_flags = [f'-DCMAKE_BUILD_TYPE={build_type}'] + extra_flags + ONLY_CLIENT
    flags_key = ' '.join(all_flags)
    flags_file = os.path.join(build_dir, 'cmake_flags.txt')

    needs_reconfigure = True
    if os.path.exists(flags_file):
        with open(flags_file) as f:
            if f.read().strip() == flags_key:
                needs_reconfigure = False

    start = time.time()
    with open(os.path.join(build_dir, 'build.log'), 'w') as log:
        if needs_reconfigure:
            r = subprocess.run(['cmake', PROJECT_ROOT, '-GNinja'] + all_flags,
                               cwd=build_dir, stdout=log, stderr=subprocess.STDOUT)
            if r.returncode != 0:
                return {'name': name, 'error': 'cmake', 'duration': time.time() - start, 'status': Status.FAIL}
            with open(flags_file, 'w') as f:
                f.write(flags_key)

        r = subprocess.run(['ninja', 'example-client'],
                           cwd=build_dir, stdout=log, stderr=subprocess.STDOUT)

    duration = time.time() - start
    if r.returncode != 0:
        return {'name': name, 'error': 'ninja', 'duration': duration, 'status': Status.FAIL}

    binary = os.path.join(build_dir, 'example-client')
    if not os.path.exists(binary):
        return {'name': name, 'error': 'not found', 'duration': duration, 'status': Status.FAIL}

    sizes = get_size(binary)
    file_size = get_file_size(binary)

    stripped = binary + '.stripped'
    subprocess.run(['strip', '--strip-all', binary, '-o', stripped],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    stripped_size = get_file_size(stripped)

    return {
        'name': name,
        'flags': extra_flags,
        'duration': duration,
        'file_size': file_size,
        'stripped_size': stripped_size,
        'sections': sizes,
        'status': Status.PASS,
        'binary': binary,
        'stripped_binary': stripped,
    }


def print_configs_table(results, failures):
    baseline = next((r for r in results if r['name'] == 'baseline'), results[0] if results else None)

    table = Table(title='Size Analysis — Build Configurations', show_header=True, header_style='bold cyan')
    table.add_column('Config', style='white', width=20)
    table.add_column('Unstripped', justify='right', style='yellow')
    table.add_column('Stripped', justify='right', style='green')
    table.add_column('vs baseline', justify='right', style='cyan')
    table.add_column('Build Time', justify='right', style='blue')

    for r in results:
        if baseline and r['name'] != 'baseline':
            delta = r['stripped_size'] - baseline['stripped_size']
            pct = delta / baseline['stripped_size'] * 100
            sign = '+' if delta >= 0 else ''
            reduction = f"{sign}{pct:.1f}%  ({sign}{format_size(abs(delta))})"
            reduction_style = 'red' if delta > 0 else 'green'
        else:
            reduction = '—'
            reduction_style = 'white'

        row_text = Text()
        table.add_row(
            r['name'],
            format_size(r['file_size']),
            format_size(r['stripped_size']),
            Text(reduction, style=reduction_style),
            f"{r['duration']:.1f}s",
        )

    console.print(table)

    if failures:
        console.print()
        fail_table = Table(title='Failed Builds', show_header=True, header_style='bold red')
        fail_table.add_column('Config', style='white')
        fail_table.add_column('Stage', style='red')
        fail_table.add_column('Duration', justify='right', style='blue')
        for fail in failures:
            fail_table.add_row(fail['name'], fail.get('error', 'unknown'), f"{fail['duration']:.1f}s")
        console.print(fail_table)


def print_options_table(option_results, baseline_size):
    table = Table(title='Size Analysis — Per-Option Impact (vs Release baseline)',
                  show_header=True, header_style='bold cyan')
    table.add_column('Option (disabled/changed)', style='white', width=30)
    table.add_column('Stripped', justify='right', style='green')
    table.add_column('Delta', justify='right')
    table.add_column('Delta %', justify='right')

    for r in option_results:
        if 'error' in r:
            table.add_row(r['option'], 'FAILED', '—', '—')
            continue
        delta = r['stripped_size'] - baseline_size
        pct = delta / baseline_size * 100
        sign = '+' if delta >= 0 else ''
        delta_str = f"{sign}{format_size(abs(delta))}" if delta != 0 else '0'
        pct_str = f"{sign}{pct:.1f}%"
        style = 'red' if delta > 0 else 'green'
        table.add_row(
            r['option'],
            format_size(r['stripped_size']),
            Text(delta_str, style=style),
            Text(pct_str, style=style),
        )

    console.print(table)


def print_bloaty_table(bloaty_rows, title):
    if not bloaty_rows:
        console.print(f'[yellow]bloaty: no data for {title}[/yellow]')
        return
    table = Table(title=title, show_header=True, header_style='bold cyan')
    table.add_column('Compile Unit', style='white', no_wrap=False)
    table.add_column('VM Size', justify='right', style='yellow')
    table.add_column('File Size', justify='right', style='green')
    for row in bloaty_rows:
        table.add_row(row['name'], format_size(row['vmsize']), format_size(row['filesize']))
    console.print(table)


def main():
    parser = argparse.ArgumentParser(description='Analyze binary sizes for different build configurations')
    parser.add_argument('--clean', action='store_true', help='Clean build directories before building')
    parser.add_argument('--no-bloaty', action='store_true', help='Skip bloaty analysis')
    args = parser.parse_args()

    if args.clean:
        console.print('Cleaning build directories...', style='yellow')
        if os.path.exists(BUILD_DIR):
            shutil.rmtree(BUILD_DIR)

    os.makedirs(BUILD_DIR, exist_ok=True)

    # --- Build main configs ---
    results, failures = [], []
    total = len(CONFIGS) + len(OPTION_CONFIGS)

    with create_progress() as progress:
        task = progress.add_task('Building', total=total)

        for cfg in CONFIGS:
            r = build_config(cfg['name'], cfg['flags'])
            (failures if 'error' in r else results).append(r)
            label = f"  {r['status']} {r['name']} ({r['duration']:.1f}s)"
            if 'error' not in r:
                label += f" — {format_size(r['stripped_size'])} stripped"
            console.print(Text(label, style=status_style(r['status'])))
            progress.advance(task)

        # --- Build per-option configs ---
        option_results = []
        for opt_cfg in OPTION_CONFIGS:
            name = f"opt-{opt_cfg['option'].lower().replace('_', '-')}"
            r = build_config(name, opt_cfg['flags'])
            if 'error' not in r:
                r['option'] = opt_cfg['option']
                option_results.append(r)
            else:
                option_results.append({'option': opt_cfg['option'], **r})
            label = f"  {r['status']} {name} ({r['duration']:.1f}s)"
            if 'error' not in r:
                label += f" — {format_size(r['stripped_size'])} stripped"
            console.print(Text(label, style=status_style(r['status'])))
            progress.advance(task)

    console.print()

    # --- Tables ---
    print_configs_table(results, failures)
    console.print()

    baseline = next((r for r in results if r['name'] == 'baseline'), None)
    if baseline and option_results:
        print_options_table(option_results, baseline['stripped_size'])
        console.print()

    # --- Bloaty ---
    if not args.no_bloaty and baseline:
        console.print('[bold cyan]Bloaty Analysis — baseline (top 20 symbols)[/bold cyan]')
        rows = run_bloaty(baseline['binary'], datasource='symbols')
        print_bloaty_table(rows, 'Baseline — symbols by file size')
        console.print()

        iot = next((r for r in results if r['name'] == 'iot-min'), None)
        if iot and 'error' not in iot:
            console.print('[bold cyan]Bloaty Analysis — iot-min vs baseline (delta, symbols)[/bold cyan]')
            rows = run_bloaty(iot['binary'], baseline['binary'], datasource='symbols')
            print_bloaty_table(rows, 'iot-min vs baseline — delta by symbol')
            console.print()

    # --- Save summary ---
    summary_file = os.path.join(BUILD_DIR, 'summary.json')
    with open(summary_file, 'w') as f:
        json.dump({'results': results, 'failures': failures, 'option_results': option_results}, f, indent=2, default=str)
    console.print(f'[dim]Results saved to {summary_file}[/dim]')


if __name__ == '__main__':
    main()
