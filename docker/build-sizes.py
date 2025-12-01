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
from rich.panel import Panel
from rich.text import Text

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(os.path.expanduser('~/.cache/SUPL-3GPP-LPP-client'), 'sizes')
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

CONFIGS = [
    {'name': 'debug', 'flags': ['-DCMAKE_BUILD_TYPE=Debug']},
    {'name': 'baseline', 'flags': ['-DCMAKE_BUILD_TYPE=Release']},
    {'name': 'size-opt', 'flags': ['-DCMAKE_CXX_FLAGS=-Os']},
    {'name': 'lto', 'flags': ['-DCMAKE_CXX_FLAGS=-Os -flto', '-DCMAKE_EXE_LINKER_FLAGS=-flto']},
    {'name': 'gc-sections', 'flags': ['-DCMAKE_CXX_FLAGS=-Os -ffunction-sections -fdata-sections', '-DCMAKE_EXE_LINKER_FLAGS=-Wl,--gc-sections']},
    {'name': 'full-opt', 'flags': ['-DCMAKE_CXX_FLAGS=-Os -flto -ffunction-sections -fdata-sections', '-DCMAKE_EXE_LINKER_FLAGS=-flto -Wl,--gc-sections']},
    {'name': 'no-logging', 'flags': ['-DDISABLE_LOGGING=ON']},
    {'name': 'no-tracing', 'flags': ['-DDISABLE_TRACE=ON']},
    {'name': 'no-spartn-tokoro', 'flags': ['-DINCLUDE_GENERATOR_SPARTN=OFF', '-DINCLUDE_GENERATOR_TOKORO=OFF']},
    {'name': 'iot-optimized', 'flags': ['-DCMAKE_CXX_FLAGS=-Os -flto -ffunction-sections -fdata-sections', '-DCMAKE_EXE_LINKER_FLAGS=-flto -Wl,--gc-sections', '-DDISABLE_LOGGING=ON', '-DDATA_TRACING=OFF', '-DINCLUDE_GENERATOR_SPARTN=OFF', '-DINCLUDE_GENERATOR_TOKORO=OFF']},
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

def build_config(config, task_id, progress):
    name = config['name']
    build_dir = os.path.join(BUILD_DIR, name)
    os.makedirs(build_dir, exist_ok=True)
    
    flags_file = os.path.join(build_dir, 'cmake_flags.txt')
    current_flags = ' '.join(config['flags'])
    needs_reconfigure = True
    
    if os.path.exists(flags_file):
        with open(flags_file, 'r') as f:
            if f.read().strip() == current_flags:
                needs_reconfigure = False
    
    progress.update(task_id, description=f"Building {name}")
    
    start = time.time()
    with open(os.path.join(build_dir, 'build.log'), 'w') as log:
        if needs_reconfigure:
            result = subprocess.run(['cmake', PROJECT_ROOT, '-GNinja', '-DCMAKE_BUILD_TYPE=Release'] + config['flags'],
                                  cwd=build_dir, stdout=log, stderr=subprocess.STDOUT)
            if result.returncode != 0:
                duration = time.time() - start
                return {'name': name, 'error': 'cmake', 'duration': duration, 'status': Status.FAIL}
            
            with open(flags_file, 'w') as f:
                f.write(current_flags)
        
        result = subprocess.run(['ninja', 'example-client'],
                              cwd=build_dir, stdout=log, stderr=subprocess.STDOUT)
    
    duration = time.time() - start
    
    if result.returncode != 0:
        return {'name': name, 'error': 'ninja', 'duration': duration, 'status': Status.FAIL}
    
    binary = os.path.join(build_dir, 'example-client')
    if not os.path.exists(binary):
        return {'name': name, 'error': 'not found', 'duration': duration, 'status': Status.FAIL}
    
    sizes = get_size(binary)
    file_size = get_file_size(binary)
    
    subprocess.run(['strip', '--strip-all', binary, '-o', binary + '.stripped'], 
                  stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    stripped_size = get_file_size(binary + '.stripped')
    
    result = {
        'name': name,
        'flags': config['flags'],
        'duration': duration,
        'file_size': file_size,
        'stripped_size': stripped_size,
        'sections': sizes,
        'status': Status.PASS
    }
    
    with open(os.path.join(build_dir, 'result.json'), 'w') as f:
        json.dump(result, f, indent=2)
    
    return result

def main():
    parser = argparse.ArgumentParser(description='Analyze binary sizes for different build configurations')
    parser.add_argument('--clean', action='store_true', help='Clean build directories before building')
    args = parser.parse_args()
    
    if args.clean:
        console.print("Cleaning build directories...", style="yellow")
        if os.path.exists(BUILD_DIR):
            shutil.rmtree(BUILD_DIR)
    
    os.makedirs(BUILD_DIR, exist_ok=True)
    
    results = []
    failures = []
    
    with create_progress() as progress:
        task = progress.add_task("Building configurations", total=len(CONFIGS))
        
        for config in CONFIGS:
            result = build_config(config, task, progress)
            if result:
                if 'error' in result:
                    failures.append(result)
                    text = Text(f"  {result['status']} {result['name']} ({result['duration']:.1f}s)")
                    text.stylize(status_style(result['status']))
                    console.print(text)
                else:
                    results.append(result)
                    text = Text(f"  {result['status']} {result['name']} ({result['duration']:.1f}s) - {format_size(result['stripped_size'])} stripped")
                    text.stylize(status_style(result['status']))
                    console.print(text)
            progress.advance(task)
    
    console.print()
    table = Table(title="Size Analysis Results", show_header=True, header_style="bold cyan")
    table.add_column("Config", style="white", width=20)
    table.add_column("Unstripped", justify="right", style="yellow")
    table.add_column("Stripped", justify="right", style="green")
    table.add_column("Reduction", justify="right", style="cyan")
    table.add_column("Build Time", justify="right", style="blue")
    
    baseline = next((r for r in results if r['name'] == 'baseline'), results[0] if results else None)
    for r in results:
        if baseline:
            pct = (1 - r['stripped_size']/baseline['stripped_size']) * 100
            reduction = f"{pct:+.1f}%"
        else:
            reduction = "-"
        
        table.add_row(
            r['name'],
            format_size(r['file_size']),
            format_size(r['stripped_size']),
            reduction,
            f"{r['duration']:.1f}s"
        )
    
    console.print(table)
    
    if failures:
        console.print()
        fail_table = Table(title="Failed Builds", show_header=True, header_style="bold red")
        fail_table.add_column("Config", style="white")
        fail_table.add_column("Stage", style="red")
        fail_table.add_column("Duration", justify="right", style="blue")
        
        for fail in failures:
            fail_table.add_row(fail['name'], fail.get('error', 'unknown'), f"{fail['duration']:.1f}s")
        
        console.print(fail_table)
    
    summary_file = os.path.join(BUILD_DIR, 'summary.json')
    with open(summary_file, 'w') as f:
        json.dump({'results': results, 'failures': failures}, f, indent=2)
    
    console.print(f"\n[dim]Results saved to {summary_file}[/dim]")

if __name__ == '__main__':
    main()
