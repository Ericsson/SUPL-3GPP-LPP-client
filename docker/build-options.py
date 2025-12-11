#!/usr/bin/env python3
import subprocess
import itertools
import os
import sys
import argparse
import time
import json
import hashlib

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CACHE_DIR = os.path.expanduser('~/.cache/SUPL-3GPP-LPP-client')
TEST_DIR = os.path.join(CACHE_DIR, 'option-tests')

EXTRA_CMAKE_ARGS = [
    '-DUNITY_BUILD=ON'
]

INDEPENDENT_GROUPS = [
    ['BUILD_TESTING', 'DATA_TRACING'],
    ['BUILD_TESTING', 'LOG_FUNCTION_PERFORMANCE'],
    ['HAVE_SPLICE', 'INCLUDE_GENERATOR_RTCM'],
    ['HAVE_SPLICE', 'INCLUDE_GENERATOR_SPARTN'],
    ['HAVE_SPLICE', 'INCLUDE_GENERATOR_TOKORO'],
    ['HAVE_SPLICE', 'INCLUDE_GENERATOR_IDOKEIDO'],
    ['HAVE_SPLICE', 'INCLUDE_FORMAT_RINEX'],
    ['HAVE_SPLICE', 'INCLUDE_FORMAT_ANTEX'],
]

OPTIONS = {
    'HAVE_SPLICE': ['ON', 'OFF'],
    'INCLUDE_GENERATOR_RTCM': ['ON', 'OFF'],
    'INCLUDE_GENERATOR_SPARTN': ['ON', 'OFF'],
    'INCLUDE_GENERATOR_TOKORO': ['ON', 'OFF'],
    'INCLUDE_GENERATOR_IDOKEIDO': ['ON', 'OFF'],
    'INCLUDE_FORMAT_RINEX': ['ON', 'OFF'],
    'INCLUDE_FORMAT_ANTEX': ['ON', 'OFF'],
    'DISABLE_LOGGING': ['ON', 'OFF'],
    'LOG_FUNCTION_PERFORMANCE': ['ON', 'OFF'],
    'DATA_TRACING': ['ON', 'OFF'],
    'BUILD_TESTING': ['ON', 'OFF'],
}

INCOMPATIBLE = [
    (['USE_ASAN=ON'], ['DATA_TRACING=ON']),
    (['INCLUDE_FORMAT_RINEX=ON'], ['INCLUDE_GENERATOR_RTCM=OFF']),
    (['INCLUDE_FORMAT_ANTEX=ON'], ['INCLUDE_GENERATOR_RTCM=OFF']),
    (['INCLUDE_GENERATOR_TOKORO=ON'], ['INCLUDE_GENERATOR_RTCM=OFF']),
    (['INCLUDE_GENERATOR_IDOKEIDO=ON'], ['INCLUDE_GENERATOR_TOKORO=OFF']),
]

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    END = '\033[0m'

def colorize(text, color):
    return f"{color}{text}{Colors.END}"

def format_time(seconds):
    if seconds < 60:
        return f"{seconds:5.1f}s"
    elif seconds < 3600:
        return f"{int(seconds // 60):2}m {int(seconds % 60):2}s"
    else:
        return f"{int(seconds // 3600)}h {int((seconds % 3600) // 60):2}m"

def is_compatible(config):
    for incompatible_set in INCOMPATIBLE:
        if all(any(opt in config for opt in group) for group in incompatible_set):
            return False
    return True

def config_to_name(config):
    return '-'.join(sorted(config))

def config_to_short_name(config):
    abbrev = {
        'USE_OPENSSL': 'ssl',
        'HAVE_SPLICE': 'splice',
        'INCLUDE_GENERATOR_RTCM': 'rtcm',
        'INCLUDE_GENERATOR_SPARTN': 'spartn',
        'INCLUDE_GENERATOR_TOKORO': 'tokoro',
        'INCLUDE_GENERATOR_IDOKEIDO': 'idokeido',
        'INCLUDE_FORMAT_RINEX': 'rinex',
        'INCLUDE_FORMAT_ANTEX': 'antex',
        'BUILD_EXAMPLES': 'examples',
        'DISABLE_LOGGING': 'nolog',
        'DISABLE_TRACE': 'notrace',
        'LOG_FUNCTION_PERFORMANCE': 'logperf',
        'DATA_TRACING': 'datatrace',
        'BUILD_TESTING': 'test',
    }
    changed = []
    for opt in config:
        if opt.endswith('=OFF'):
            key = opt[:-4]
            if key in abbrev:
                changed.append(f"no-{abbrev[key]}")
        elif opt.endswith('=ON'):
            key = opt[:-3]
            if key in abbrev:
                changed.append(abbrev[key])
    if not changed:
        return 'all-defaults'
    return ' '.join(changed)

def test_configuration(config, compiler, build_type, cxx_std, jobs, current, total):
    project_root = os.path.dirname(SCRIPT_DIR)
    config_name = config_to_name(config)
    config_short = config_to_short_name(config)
    config_hash = hashlib.sha256(config_name.encode()).hexdigest()[:16]
    test_name = f"gcc48-{build_type}-cxx{cxx_std}-{config_hash}"
    
    width = len(str(total))
    progress = f"[{current:>{width}}/{total}]"
    
    if not is_compatible(config):
        print(f"[{colorize('SKIP', Colors.YELLOW)}] {progress} {format_time(0)} {config_short}")
        return {'result': 'SKIP', 'duration': 0}
    
    build_dir = os.path.join(TEST_DIR, test_name)
    os.makedirs(build_dir, exist_ok=True)
    
    with open(os.path.join(build_dir, 'config.txt'), 'w') as f:
        f.write(config_name + '\n')
    
    cmake_options = ' '.join(f'-D{opt}' for opt in config)
    extra_args = ' '.join(EXTRA_CMAKE_ARGS)
    cmake_cmd = f"mkdir -p /build && cd /build && cmake /src -GNinja -DCMAKE_BUILD_TYPE={build_type} -DCMAKE_CXX_STANDARD={cxx_std} {cmake_options} {extra_args} && ninja -j{jobs}"
    
    print(f"[....] {progress} {format_time(0)} {config_short}", end='', flush=True)
    start_time = time.time()
    
    try:
        with open(os.path.join(build_dir, 'build.log'), 'w') as log:
            log.write(f"Config: {config_name}\n")
            log.write(f"Running: {cmake_cmd}\n")
            result = subprocess.run([
                'docker', 'run', '--rm',
                '--user', f'{os.getuid()}:{os.getgid()}',
                '-v', f'{project_root}:/src:ro',
                '-v', f'{os.path.abspath(build_dir)}:/build',
                '-w', '/build',
                '--network=host',
                's3lc-test:gcc-4.8',
                'bash', '-c', cmake_cmd
            ], stdout=log, stderr=subprocess.STDOUT)
        
        duration = time.time() - start_time
        
        if result.returncode == 0:
            print(f"\r[{colorize('PASS', Colors.GREEN)}] {progress} {format_time(duration)} {config_short}")
            subprocess.run(['rm', '-rf', build_dir], check=False)
            return {'result': True, 'duration': duration}
        else:
            print(f"\r[{colorize('FAIL', Colors.RED)}] {progress} {format_time(duration)} {config_short}")
            print(f"  Log: {os.path.join(build_dir, 'build.log')}")
            with open(os.path.join(build_dir, 'build.log'), 'r') as log:
                lines = log.readlines()
                error_lines = [l for l in lines if 'error:' in l.lower() or 'fatal' in l.lower()]
                if error_lines:
                    print(f"  Errors:")
                    for line in error_lines[:5]:
                        print(f"    {line.rstrip()}")
            return {'result': False, 'duration': duration}
    except Exception as e:
        duration = time.time() - start_time
        print(f"\r[{colorize('FAIL', Colors.RED)}] {progress} {format_time(duration)} {config_short}")
        print(f"  Exception: {e}")
        with open(os.path.join(build_dir, 'build.log'), 'a') as log:
            log.write(f"\nTest failed: {e}\n")
        return {'result': False, 'duration': duration}

def generate_combinations(options_dict):
    if not INDEPENDENT_GROUPS:
        keys = list(options_dict.keys())
        values = [options_dict[k] for k in keys]
        for combination in itertools.product(*values):
            yield [f"{k}={v}" for k, v in zip(keys, combination)]
        return
    
    keys = list(options_dict.keys())
    values = [options_dict[k] for k in keys]
    
    for combination in itertools.product(*values):
        config = dict(zip(keys, combination))
        
        skip = False
        for group in INDEPENDENT_GROUPS:
            on_count = sum(1 for k in group if config[k] == 'ON')
            if on_count > 1:
                skip = True
                break
        
        if not skip:
            yield [f"{k}={v}" for k, v in config.items()]

def main():
    parser = argparse.ArgumentParser(description='Test all option combinations with gcc-4.8')
    parser.add_argument('--build-type', '-b', default='Debug', choices=['Debug', 'Release'])
    parser.add_argument('--cxx-std', '-s', default='11', choices=['11'])
    parser.add_argument('--jobs', '-j', type=int, default=1)
    parser.add_argument('--limit', '-l', type=int, help='Limit number of tests')
    args = parser.parse_args()
    
    os.makedirs(TEST_DIR, exist_ok=True)
    
    all_configs = list(generate_combinations(OPTIONS))
    total = args.limit if args.limit else len(all_configs)
    
    print(f"{colorize('Testing', Colors.BOLD)} {total} configuration(s)\n")
    
    results = []
    total_start = time.time()
    
    for i, config in enumerate(all_configs, 1):
        if args.limit and i > args.limit:
            break
        
        result = test_configuration(config, 'gcc-4.8', args.build_type, args.cxx_std, args.jobs, i, total)
        results.append({'config': config, **result})
    
    total_time = time.time() - total_start
    
    passed = sum(1 for r in results if r['result'] is True)
    failed = sum(1 for r in results if r['result'] is False)
    skipped = sum(1 for r in results if r['result'] == 'SKIP')
    
    print(f"\n{colorize('='*80, Colors.BOLD)}")
    print(f"{colorize('SUMMARY:', Colors.BOLD)} {colorize(str(passed), Colors.GREEN)} passed, {colorize(str(failed), Colors.RED)} failed, {colorize(str(skipped), Colors.YELLOW)} skipped ({len(results)} total)")
    print(f"{colorize('TOTAL TIME:', Colors.BOLD)} {format_time(total_time)}")
    
    if failed > 0:
        print(f"\n{colorize('FAILED CONFIGURATIONS:', Colors.RED)}")
        for r in results:
            if r['result'] is False:
                print(f"  {config_to_name(r['config'])}")
        sys.exit(1)

if __name__ == '__main__':
    main()
