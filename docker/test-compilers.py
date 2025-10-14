#!/usr/bin/env python3
import subprocess
import itertools
import os
import sys
import argparse
import re
import signal
import time
import json

# Configuration paths
DOCKER_DIR = 'docker'
IMAGE_DIR = f'{DOCKER_DIR}/build/images'
COMPILER_DIR = f'{DOCKER_DIR}/build/compilers'
TEST_DIR = f'{DOCKER_DIR}/build/tests'

IMAGES = {
    's3lc-base:18.04': 'Dockerfile.base-18.04',
    's3lc-base:22.04': 'Dockerfile.base-22.04',
    's3lc-base:24.04': 'Dockerfile.base-24.04',
}

COMPILERS = {
    'gcc-4.8': {
        'image': 's3lc-base:18.04',
        'package': 'gcc-4.8 g++-4.8',
        'setup': 'RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8',
        'cxx_standards': ['11'],
    },
    'gcc-7': {
        'image': 's3lc-base:18.04',
        'package': 'gcc-7 g++-7',
        'setup': 'RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 --slave /usr/bin/g++ g++ /usr/bin/g++-7',
        'cxx_standards': ['11', '14', '17'],
    },
    'gcc-9': {
        'image': 's3lc-base:18.04',
        'package': 'gcc-9 g++-9',
        'setup': 'RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9',
        'cxx_standards': ['11', '14', '17'],
    },
    'gcc-11': {
        'image': 's3lc-base:22.04',
        'package': 'gcc-11 g++-11',
        'setup': 'RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 60 --slave /usr/bin/g++ g++ /usr/bin/g++-11',
        'cxx_standards': ['11', '14', '17', '20'],
    },
    'gcc-13': {
        'image': 's3lc-base:22.04',
        'package': 'gcc-13 g++-13',
        'setup': 'RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 60 --slave /usr/bin/g++ g++ /usr/bin/g++-13',
        'cxx_standards': ['11', '14', '17', '20'],
    },
    'gcc-14': {
        'image': 's3lc-base:24.04',
        'package': 'gcc-14 g++-14',
        'setup': 'RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 60 --slave /usr/bin/g++ g++ /usr/bin/g++-14',
        'cxx_standards': ['11', '14', '17', '20'],
    },
    'clang-10': {
        'image': 's3lc-base:18.04',
        'package': 'clang-10',
        'setup': 'RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-10 60 --slave /usr/bin/c++ c++ /usr/bin/clang++-10',
        'cxx_standards': ['11', '14', '17'],
    },
    'clang-15': {
        'image': 's3lc-base:22.04',
        'package': 'clang-15',
        'setup': 'RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-15 60 --slave /usr/bin/c++ c++ /usr/bin/clang++-15',
        'cxx_standards': ['11', '14', '17', '20'],
    },
    'clang-18': {
        'image': 's3lc-base:24.04',
        'package': 'clang-18',
        'setup': 'RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-18 60 --slave /usr/bin/c++ c++ /usr/bin/clang++-18',
        'cxx_standards': ['11', '14', '17', '20'],
    },
    'clang-19': {
        'image': 's3lc-base:24.04',
        'package': 'clang-19',
        'setup': 'RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-19 60 --slave /usr/bin/c++ c++ /usr/bin/clang++-19',
        'cxx_standards': ['11', '14', '17', '20'],
    },
}

MATRIX = {
    'build_types': ['Debug', 'Release'],
    'cxx_standards': ['11', '17', '20'],
    'cmake_options': [
        [],
        ['-DUSE_OPENSSL=ON'],
        ['-DUSE_ASAN=ON'],
        ['-DDISABLE_LOGGING=ON'],
        ['-DDATA_TRACING=ON'],
        ['-DINCLUDE_GENERATOR_IDOKEIDO=ON'],
    ],
    'incompatible': {
        'gcc-4.8': ['-DUSE_ASAN=ON', '-DDATA_TRACING=ON'],
        'gcc-7': ['-DDATA_TRACING=ON'],
        'gcc-9': ['-DDATA_TRACING=ON'],
        'clang-10': ['-DDATA_TRACING=ON'],
    }
}

# Colors
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
        return f"{seconds:.1f}s"
    elif seconds < 3600:
        return f"{int(seconds // 60)}m {int(seconds % 60)}s"
    else:
        return f"{int(seconds // 3600)}h {int((seconds % 3600) // 60)}m"

def load_cached_result(test_dir):
    result_file = f'{test_dir}/result.json'
    if os.path.exists(result_file):
        try:
            with open(result_file, 'r') as f:
                return json.load(f)
        except:
            return None
    return None

def save_result(test_dir, result, duration):
    result_file = f'{test_dir}/result.json'
    with open(result_file, 'w') as f:
        json.dump({'result': result, 'duration': duration, 'timestamp': time.time()}, f)

def image_name_to_path(image_name):
    return image_name.replace(':', '_').replace('/', '_')

def build_base_images(results):
    for image_name, dockerfile in IMAGES.items():
        print(f"Building base image {image_name}...")
        image_dir = f'{IMAGE_DIR}/{image_name_to_path(image_name)}'
        os.makedirs(image_dir, exist_ok=True)
        try:
            with open(f'{image_dir}/build.log', 'w') as log:
                log.write(f"Building base image {image_name}...\n")
                subprocess.run(['docker', 'build', '-f', f'{DOCKER_DIR}/{dockerfile}', '-t', image_name, '.', '--network=host'], 
                             stdout=log, stderr=subprocess.STDOUT, check=True)
            results['images'][image_name] = True
        except Exception as e:
            results['images'][image_name] = False
            with open(f'{image_dir}/build.log', 'w') as log:
                log.write(f"Base build failed: {e}\n")
            return False
    return True

def generate_dockerfile(compiler, compiler_config):
    with open(f'{DOCKER_DIR}/Dockerfile.template', 'r') as f:
        template = f.read()
    
    compiler_file = f'{COMPILER_DIR}/Dockerfile.{compiler}'
    dockerfile = template.replace('{{BASE_IMAGE}}', compiler_config['image']).replace('{{COMPILER_PACKAGE}}', compiler_config['package']).replace('{{COMPILER_SETUP}}', compiler_config['setup'])
    
    with open(compiler_file, 'w') as f:
        f.write(dockerfile)

def build_compiler_image(compiler, results):
    print(f"Building {compiler}...")
    compiler_dir = f'{COMPILER_DIR}/{compiler}'
    os.makedirs(compiler_dir, exist_ok=True)
    try:
        with open(f'{compiler_dir}/build.log', 'w') as log:
            log.write(f"Building {compiler}...\n")
            result = subprocess.run(['docker', 'build', '-f', f'{COMPILER_DIR}/Dockerfile.{compiler}', '-t', f's3lc-test:{compiler}', '.', '--network=host'], 
                                  stdout=log, stderr=subprocess.STDOUT)
        results['compilers'][compiler] = result.returncode == 0
        return result.returncode == 0
    except Exception as e:
        results['compilers'][compiler] = False
        with open(f'{compiler_dir}/build.log', 'w') as log:
            log.write(f"Build failed with exception: {e}\n")
        return False

def is_compatible(compiler, options):
    incompatible = MATRIX.get('incompatible', {}).get(compiler, [])
    return not any(opt in incompatible for opt in options)

def test_name_from_config(compiler, build_type, cxx_std, options):
    return f"{compiler}-{build_type}-cxx{cxx_std}-{'-'.join(opt.replace('=', '-').replace('-D', '') for opt in options) if options else 'default'}"

def test_configuration(compiler, build_type, cxx_std, options, results, force_rebuild):
    config = test_name_from_config(compiler, build_type, cxx_std, options)
    
    test_data = {
        'compiler': compiler,
        'build_type': build_type,
        'cxx_std': cxx_std,
        'options': options,
        'result': None,
        'duration': 0
    }

    prefix = f"Testing {config}..."
    print(f"{prefix:<80} ", end='', flush=True)

    if cxx_std not in COMPILERS[compiler]['cxx_standards']:
        print(colorize("SKIP", Colors.YELLOW))
        test_data['result'] = 'SKIP'
        results['tests'][config] = test_data
        return
        
    if not is_compatible(compiler, options):
        print(colorize("SKIP", Colors.YELLOW))
        test_data['result'] = 'SKIP'
        results['tests'][config] = test_data
        return
    
    build_dir = f'{TEST_DIR}/{config}'
    os.makedirs(build_dir, exist_ok=True)
    
    # Check for cached result
    if not force_rebuild:
        cached = load_cached_result(build_dir)
        if cached and os.path.exists(f'{build_dir}/build.log'):
            test_data['result'] = cached.get('result')
            test_data['duration'] = cached.get('duration', 0)
            if test_data['result']:
                print(colorize(f"PASS (cached, {format_time(test_data['duration'])})", Colors.GREEN))
            else:
                print(colorize(f"FAIL (cached, {format_time(test_data['duration'])})", Colors.RED))
            results['tests'][config] = test_data
            return
    
    cmake_cmd = f"mkdir -p /build && cd /build && cmake /src -GNinja -DCMAKE_BUILD_TYPE={build_type} -DCMAKE_CXX_STANDARD={cxx_std} {' '.join(options)} && ninja"
    
    start_time = time.time()
    try:
        with open(f'{build_dir}/build.log', 'w') as log:
            log.write(f"Running: {cmake_cmd}\n")
            result = subprocess.run(['docker', 'run', '--rm', 
                                   '-v', f'{os.getcwd()}:/src:ro',
                                   '-v', f'{os.path.abspath(build_dir)}:/build',
                                   '-w', '/build', 
                                   '--network=host',
                                   f's3lc-test:{compiler}', 'bash', '-c', cmake_cmd], 
                                  stdout=log, stderr=subprocess.STDOUT)
        duration = time.time() - start_time
        test_data['duration'] = duration
        
        if result.returncode == 0:
            print(colorize(f"PASS ({format_time(duration)})", Colors.GREEN))
            test_data['result'] = True
        else:
            print(colorize(f"FAIL ({format_time(duration)})", Colors.RED))
            test_data['result'] = False
        
        save_result(build_dir, test_data['result'], duration)
    except Exception as e:
        duration = time.time() - start_time
        test_data['duration'] = duration
        print(colorize(f"FAIL ({format_time(duration)})", Colors.RED))
        test_data['result'] = False
        with open(f'{build_dir}/build.log', 'a') as log:
            log.write(f"\nTest failed with exception: {e}\n")
        save_result(build_dir, False, duration)
    
    results['tests'][config] = test_data

def print_results_matrix(results):
    print("\n" + colorize("="*80, Colors.BOLD))
    print(colorize("TEST RESULTS MATRIX", Colors.BOLD))
    print(colorize("="*80, Colors.BOLD))
    
    # Images
    print(colorize("\nIMAGES:", Colors.BLUE))
    for image, result in sorted(results.get('images', {}).items()):
        if result:
            status = colorize("PASS", Colors.GREEN)
        else:
            status = colorize("FAIL", Colors.RED)
        print(f"  {image:<40} {status}")
    
    # Compilers
    print(colorize("\nCOMPILERS:", Colors.BLUE))
    for compiler, result in sorted(results.get('compilers', {}).items()):
        if result:
            status = colorize("PASS", Colors.GREEN)
        else:
            status = colorize("FAIL", Colors.RED)
        print(f"  {compiler:<40} {status}")
    
    # Tests Grid
    print(colorize("\nTESTS:", Colors.BLUE))
    tests = results.get('tests', {})
    if tests:
        # Extract all compiler/build combinations and C++/option combinations
        compiler_builds = set()
        cxx_options = set()
        
        for test_data in tests.values():
            compiler = test_data['compiler']
            build_type = test_data['build_type']
            cxx_std = test_data['cxx_std']
            options = test_data['options']
            
            # Get compiler display name
            if compiler.startswith('gcc-'):
                comp_name = f"GCC {compiler[4:]}"
            elif compiler.startswith('clang-'):
                comp_name = f"Clang {compiler[6:]}"
            else:
                comp_name = compiler
            
            compiler_builds.add(f"{comp_name} {build_type}")
            
            if not options:
                cxx_options.add(f"C++{cxx_std}")
            else:
                opt_str = ' '.join(opt.replace('-D', '').replace('=ON', '') for opt in options)
                cxx_options.add(f"C++{cxx_std} {opt_str}")
        
        compiler_builds = sorted(compiler_builds)
        cxx_options = sorted(cxx_options)
        
        if compiler_builds and cxx_options:
            # Print header
            header = f"\n  {'':<40}"
            for cb in compiler_builds:
                top = ' '.join(cb.split(' ')[:-1])
                header += f"{top:<12}"
            print(header)
            header = f"  {'C++/Options':<40}"
            for cb in compiler_builds:
                bottom = cb.split(' ')[-1]
                header += f"{bottom:<12}"
            print(header)
            print(f"  {'-' * len(header)}")
            
            # Print rows
            for cxx_opt in cxx_options:
                row = f"  {cxx_opt:<40}"
                
                for comp_build in compiler_builds:
                    # Find matching test result
                    result = None
                    for test_data in tests.values():
                        compiler = test_data['compiler']
                        build_type = test_data['build_type']
                        cxx_std = test_data['cxx_std']
                        options = test_data['options']
                        
                        # Get compiler display name
                        if compiler.startswith('gcc-'):
                            comp_name = f"GCC {compiler[4:]}"
                        elif compiler.startswith('clang-'):
                            comp_name = f"Clang {compiler[6:]}"
                        else:
                            comp_name = compiler
                        
                        config_comp_build = f"{comp_name} {build_type}"
                        if not options:
                            config_cxx_opt = f"C++{cxx_std}"
                        else:
                            opt_str = ' '.join(opt.replace('-D', '').replace('=ON', '') for opt in options)
                            config_cxx_opt = f"C++{cxx_std} {opt_str}"
                        
                        if config_comp_build == comp_build and config_cxx_opt == cxx_opt:
                            result = test_data['result']
                            break
                    
                    if result is True:
                        status = colorize("PASS", Colors.GREEN)
                    elif result is False:
                        status = colorize("FAIL", Colors.RED)
                    elif result == 'SKIP':
                        status = colorize("SKIP", Colors.YELLOW)
                    else:
                        status = colorize("----", Colors.BLUE)
                    row += f"{status:<21}"
                print(row)
    
    # Summary
    all_results = []
    for category in results.values():
        if isinstance(category, dict):
            all_results.extend(category.values())
    
    test_results = [test_data['result'] for test_data in results.get('tests', {}).values()]
    passed = sum(1 for v in test_results if v is True)
    failed = sum(1 for v in test_results if v is False)
    skipped = sum(1 for v in test_results if v == 'SKIP')
    
    # Calculate total time
    total_time = sum(test_data.get('duration', 0) for test_data in results.get('tests', {}).values())
    
    # Add other results
    for category in ['images', 'compilers']:
        if category in results:
            all_results.extend(results[category].values())
    
    passed += sum(1 for v in all_results if v is True)
    failed += sum(1 for v in all_results if v is False)
    total = len(test_results) + len(all_results)
    total = len(all_results)
    
    print(f"\n{colorize('OVERALL:', Colors.BOLD)} {colorize(str(passed), Colors.GREEN)} passed, {colorize(str(failed), Colors.RED)} failed, {colorize(str(skipped), Colors.YELLOW)} skipped ({total} total)")
    print(f"{colorize('TOTAL TIME:', Colors.BOLD)} {format_time(total_time)}")
    print(f"Log files saved in build directories")
    
    if failed > 0:
        print(f"\n{colorize(str(failed), Colors.RED)} tests failed. Check log files for details.")
        sys.exit(1)

def matches_filter(config, filters):
    if not filters:
        return True
    return any(re.search(f, config, re.IGNORECASE) for f in filters)

def main():
    parser = argparse.ArgumentParser(description='Run test matrix for SUPL 3GPP LPP client')
    parser.add_argument('--filter', '-f', action='append', help='Filter tests by regex pattern (can be used multiple times)')
    parser.add_argument('--compiler', '-c', action='append', help='Run tests only for specific compilers')
    parser.add_argument('--build-type', '-b', choices=['Debug', 'Release'], help='Run tests only for specific build type')
    parser.add_argument('--cxx-std', '-s', choices=['11', '14', '17', '20'], help='Run tests only for specific C++ standard')
    parser.add_argument('--force', action='store_true', help='Force rebuild all tests, ignore cached results')
    args = parser.parse_args()
    
    os.makedirs(IMAGE_DIR, exist_ok=True)
    os.makedirs(COMPILER_DIR, exist_ok=True)
    os.makedirs(TEST_DIR, exist_ok=True)
    
    results = {'images': {}, 'compilers': {}, 'tests': {}}
    start_time = time.time()
    
    def signal_handler(sig, frame):
        print(colorize("\n\nTest interrupted by user (CTRL+C)", Colors.YELLOW))
        print_results_matrix(results)
        sys.exit(130)
    
    signal.signal(signal.SIGINT, signal_handler)
    
    if not build_base_images(results):
        print(colorize("Base image build failed, cannot continue", Colors.RED))
        print_results_matrix(results)
        return
    
    # Filter compilers
    compilers_to_test = args.compiler if args.compiler else COMPILERS.keys()
    build_types = [args.build_type] if args.build_type else MATRIX['build_types']
    cxx_standards = [args.cxx_std] if args.cxx_std else MATRIX['cxx_standards']
    
    for compiler in compilers_to_test:
        if compiler not in COMPILERS:
            print(colorize(f"Unknown compiler: {compiler}", Colors.RED))
            continue
            
        compiler_config = COMPILERS[compiler]
        generate_dockerfile(compiler, compiler_config)
        if not build_compiler_image(compiler, results):
            print(colorize(f"Skipping tests for {compiler} due to build failure", Colors.YELLOW))
            continue
            
        for build_type, cxx_std, options in itertools.product(
            build_types, 
            cxx_standards, 
            MATRIX['cmake_options']
        ):
            config = test_name_from_config(compiler, build_type, cxx_std, options)
            if matches_filter(config, args.filter):
                test_configuration(compiler, build_type, cxx_std, options, results, args.force)
    
    total_time = time.time() - start_time
    print(f"\n{colorize('TOTAL ELAPSED TIME:', Colors.BOLD)} {format_time(total_time)}")
    print_results_matrix(results)

if __name__ == '__main__':
    main()