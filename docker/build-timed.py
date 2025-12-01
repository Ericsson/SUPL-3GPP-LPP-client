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
BUILD_DIR = os.path.join(os.path.expanduser('~/.cache/SUPL-3GPP-LPP-client'), 'timed')
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

CONFIGS = [
    {'name': 'debug', 'flags': ['-DCMAKE_BUILD_TYPE=Debug']},
    {'name': 'debug-unity', 'flags': ['-DCMAKE_BUILD_TYPE=Debug', '-DUNITY_BUILD=ON']},
]

def format_time(seconds):
    if seconds < 60:
        return f"{seconds:5.1f}s"
    elif seconds < 3600:
        return f"{int(seconds // 60):2}m {int(seconds % 60):2}s"
    else:
        return f"{int(seconds // 3600)}h {int((seconds % 3600) // 60):2}m"

def build_config(config, run_number):
    name = config['name']
    build_dir = os.path.join(BUILD_DIR, name)
    
    # Always clean before build
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    os.makedirs(build_dir, exist_ok=True)
    
    start = time.time()
    with open(os.path.join(build_dir, f'build-{run_number}.log'), 'w') as log:
        result = subprocess.run(['cmake', PROJECT_ROOT, '-GNinja'] + config['flags'],
                              cwd=build_dir, stdout=log, stderr=subprocess.STDOUT)
        if result.returncode != 0:
            duration = time.time() - start
            text = Text(f"  {Status.FAIL} {name} (run {run_number}/3) {format_time(duration)}")
            text.stylize("red")
            console.print(text)
            return None
        
        result = subprocess.run(['ninja'],
                              cwd=build_dir, stdout=log, stderr=subprocess.STDOUT)
    
    duration = time.time() - start
    
    if result.returncode != 0:
        text = Text(f"  {Status.FAIL} {name} (run {run_number}/3) {format_time(duration)}")
        text.stylize("red")
        console.print(text)
        return None
    
    text = Text(f"  {Status.PASS} {name} (run {run_number}/3) {format_time(duration)}")
    text.stylize("green")
    console.print(text)
    return duration

def main():
    parser = argparse.ArgumentParser(description='Measure build times for different configurations')
    parser.add_argument('--filter', '-f', action='append', help='Filter configs by regex pattern')
    args = parser.parse_args()
    
    os.makedirs(BUILD_DIR, exist_ok=True)
    
    configs = CONFIGS
    if args.filter:
        import re
        configs = [c for c in CONFIGS if any(re.search(f, c['name'], re.IGNORECASE) for f in args.filter)]
    
    results = {}
    total_start = time.time()
    
    with create_progress() as progress:
        task = progress.add_task("Running timed builds", total=len(configs) * 3)
        
        for config in configs:
            name = config['name']
            times = []
            
            for run in range(1, 4):
                duration = build_config(config, run)
                if duration is None:
                    progress.advance(task, 3 - run + 1)
                    break
                times.append(duration)
                progress.advance(task)
            
            if len(times) == 3:
                results[name] = {
                    'times': times,
                    'min': min(times[1:]),
                    'max': max(times[1:]),
                    'avg': sum(times[1:]) / 2,
                    'first': times[0]
                }
    
    total_time = time.time() - total_start
    
    console.print()
    table = Table(title="Build Time Analysis", show_header=True, header_style="bold cyan")
    table.add_column("Config", style="white", width=20)
    table.add_column("First", justify="right", style="yellow")
    table.add_column("Min", justify="right", style="green")
    table.add_column("Max", justify="right", style="red")
    table.add_column("Avg", justify="right", style="blue")
    
    for name, data in results.items():
        table.add_row(
            name,
            format_time(data['first']),
            format_time(data['min']),
            format_time(data['max']),
            format_time(data['avg'])
        )
    
    console.print(table)
    console.print(f"\n[bold]TOTAL TIME:[/bold] {format_time(total_time)}")
    
    summary_file = os.path.join(BUILD_DIR, 'summary.json')
    with open(summary_file, 'w') as f:
        json.dump(results, f, indent=2)
    console.print(f"[dim]Results saved to {summary_file}[/dim]")

if __name__ == '__main__':
    main()
