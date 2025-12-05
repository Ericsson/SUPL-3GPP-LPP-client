#!/usr/bin/env python3
import subprocess
import argparse
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

PLATFORMS = {
    'x86_64-unknown-linux-gnu': {
        'platform': 'linux/amd64',
        'builder': 'linux/amd64/unknown.builder',
        'runtime': 'linux/amd64/unknown.runtime',
    },
    'aarch64-unknown-linux-gnu': {
        'platform': 'linux/arm64',
        'cross': 'linux/arm64/aarch64-unknown-linux-gnu.config',
        'toolchain': '/src/docker/linux/arm64/aarch64-unknown-linux-gnu.toolchain.cmake',
        'runtime': 'linux/arm64/unknown.runtime',
    },
    'aarch64-rpi4-linux-gnu': {
        'platform': 'linux/arm64',
        'cross': 'linux/arm64/aarch64-rpi4-linux-gnu.config',
        'toolchain': '/src/docker/linux/arm64/aarch64-rpi4-linux-gnu.toolchain.cmake',
        'runtime': 'linux/arm64/rpi4.runtime',
    },
    'arm-cortex_a8-linux-gnueabi': {
        'platform': 'linux/arm/v7',
        'cross': 'linux/arm-v7/arm-cortex_a8-linux-gnueabi.config',
        'toolchain': '/src/docker/linux/arm-v7/arm-cortex_a8-linux-gnueabi.toolchain.cmake',
        'runtime': 'linux/arm-v7/cortex_a8.runtime',
    },
    'armv8-rpi3-linux-gnueabihf': {
        'platform': 'linux/arm/v7',
        'cross': 'linux/arm-v7/armv8-rpi3-linux-gnueabihf.config',
        'toolchain': '/src/docker/linux/arm-v7/armv8-rpi3-linux-gnueabihf.toolchain.cmake',
        'runtime': 'linux/arm-v7/rpi3.runtime',
        'cmake_args': '-DUSE_ASAN=OFF -DENABLE_TOKORO_BASELINE_RECORDING=ON',
    },
    'armv6-unknown-linux-gnueabihf': {
        'platform': 'linux/arm/v6',
        'cross': 'linux/arm-v6/armv6-unknown-linux-gnueabihf.config',
        'toolchain': '/src/docker/linux/arm-v6/armv6-unknown-linux-gnueabihf.toolchain.cmake',
        'runtime': 'linux/arm-v6/unknown.runtime',
    },
}

APPS = {
    'client': {
        'dockerfile': 'Dockerfile.release.client',
        'target': 'example-client',
    },
    'modem-ctrl': {
        'dockerfile': 'Dockerfile.release.modem-ctrl',
        'target': 'example-modem-ctrl',
    },
}

BUILD_MODES = ['debug', 'release']

NETWORK_HOST = False

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    END = '\033[0m'

def colorize(text, color):
    return f"{color}{text}{Colors.END}"

def run_cmd(cmd, check=True):
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, check=check)
    return result.returncode == 0

def image_exists(image_name):
    """Check if a Docker image exists locally"""
    result = subprocess.run(
        ['docker', 'image', 'inspect', image_name],
        capture_output=True,
        check=False
    )
    return result.returncode == 0

def generate_tag(arch, build_mode):
    """Generate tag from git commit, dirty state, and version"""
    try:
        commit = subprocess.run(
            ['git', 'rev-parse', '--short', 'HEAD'],
            capture_output=True, text=True, cwd=PROJECT_ROOT
        ).stdout.strip()
        
        dirty = subprocess.run(
            ['git', 'status', '--porcelain'],
            capture_output=True, text=True, cwd=PROJECT_ROOT
        ).stdout.strip()
        
        version = subprocess.run(
            ['git', 'describe', '--exact-match', '--tags'],
            capture_output=True, text=True, cwd=PROJECT_ROOT
        ).stdout.strip()
        
        if version and not dirty:
            tag = version
        else:
            tag = commit
            if dirty:
                tag += "-dirty"
        
        return tag
    except:
        return "latest"

def get_image_info(image_name):
    """Get image size"""
    try:
        result = subprocess.run(
            ['docker', 'images', image_name, '--format', '{{.Size}}'],
            capture_output=True, text=True
        )
        return result.stdout.strip() or 'N/A'
    except:
        return 'N/A'

def build_builder_image(platform):
    platform_config = PLATFORMS[platform]
    
    # Build crosstool builder for cross-compilation platforms
    if platform_config.get('cross'):
        print(f"\n{colorize('===', Colors.CYAN)} {colorize(f'Building crosstool builder for {platform}', Colors.BOLD)} {colorize('===', Colors.CYAN)}")
        
        config_path = platform_config['cross']
        
        cmd = ['docker', 'build']
        if NETWORK_HOST:
            cmd.extend(['--network=host'])
        cmd.extend([
            '--build-arg', f'PLATFORM={platform}',
            '--build-arg', f'CONFIG_PATH={config_path}',
            '-f', os.path.join(SCRIPT_DIR, 'Dockerfile.crosstool'),
            '-t', f's3lc-builder:{platform}',
            SCRIPT_DIR
        ])
        
        return run_cmd(cmd)
    
    print(f"\n{colorize('===', Colors.CYAN)} {colorize(f'Building builder for {platform}', Colors.BOLD)} {colorize('===', Colors.CYAN)}")
    
    dockerfile = platform_config.get('builder', 'Dockerfile.builder')
    
    cmd = ['docker', 'build']
    if NETWORK_HOST:
        cmd.extend(['--network=host'])
    cmd.extend(['--platform', platform_config['platform']])
    cmd.extend([
        '-f', os.path.join(SCRIPT_DIR, dockerfile),
        '-t', f's3lc-builder:{platform}',
        SCRIPT_DIR
    ])
    
    return run_cmd(cmd)

def build_image(app, platform, build_mode, registry=None, tag=None, built_artifacts=None):
    print(f"\n{colorize('===', Colors.CYAN)} {colorize(f'Building {app} for {platform} ({build_mode})', Colors.BOLD)} {colorize('===', Colors.CYAN)}")
    
    app_config = APPS[app]
    platform_config = PLATFORMS[platform]
    
    if not tag:
        tag = generate_tag(platform, build_mode)
    
    if build_mode == 'debug':
        tag += '-debug'
    
    builder_base = f's3lc-builder:{platform}'
    artifact_image = f's3lc-artifact:{tag}-{platform}'
    
    if built_artifacts is None:
        built_artifacts = set()
    
    lib_size = 0
    
    if artifact_image not in built_artifacts:
        git_commit = subprocess.run(
            ['git', 'rev-parse', '--short', 'HEAD'],
            capture_output=True, text=True, cwd=PROJECT_ROOT
        ).stdout.strip() or 'unknown'
        
        git_branch = subprocess.run(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            capture_output=True, text=True, cwd=PROJECT_ROOT
        ).stdout.strip() or 'unknown'
        
        git_dirty = '1' if subprocess.run(
            ['git', 'status', '--porcelain'],
            capture_output=True, text=True, cwd=PROJECT_ROOT
        ).stdout.strip() else '0'
        
        build_dockerfile = f'Dockerfile.build.{build_mode.lower()}'
        cmd_artifact = ['docker', 'build']
        if NETWORK_HOST:
            cmd_artifact.extend(['--network=host'])
        if not platform_config.get('cross'):
            cmd_artifact.extend(['--platform', platform_config['platform']])
        cmd_artifact.extend([
            '--build-arg', f'BUILDER_IMAGE={builder_base}',
            '--build-arg', f'BUILD_CACHE_ID=s3lc-cache-{build_mode}-{platform}',
            '--build-arg', f'GIT_COMMIT_HASH={git_commit}',
            '--build-arg', f'GIT_BRANCH={git_branch}',
            '--build-arg', f'GIT_DIRTY={git_dirty}',
        ])
        if 'toolchain' in platform_config:
            cmd_artifact.extend(['--build-arg', f'TOOLCHAIN_FILE={platform_config["toolchain"]}'])
        if 'cmake_args' in platform_config:
            cmd_artifact.extend(['--build-arg', f'CMAKE_ARGS={platform_config["cmake_args"]}'])
        cmd_artifact.extend([
            '-f', os.path.join(SCRIPT_DIR, build_dockerfile),
            '-t', artifact_image,
            PROJECT_ROOT
        ])
        
        if not run_cmd(cmd_artifact):
            return None
        built_artifacts.add(artifact_image)
    
    import tempfile
    import shutil
    artifact_dir = tempfile.mkdtemp(prefix='s3lc-artifacts-')
    try:
        container_id = subprocess.run(
            ['docker', 'create', artifact_image],
            capture_output=True, text=True, check=True
        ).stdout.strip()
        
        # Copy only the artifacts directory (contains built executables)
        subprocess.run(
            ['docker', 'cp', f'{container_id}:/artifacts/.', artifact_dir],
            check=True
        )
        
        tuple_name = None
        if platform_config.get('cross'):
            tuple_name = subprocess.run(
                ['docker', 'run', '--rm', artifact_image, 'sh', '-c', 'basename /home/builder/x-tools/*'],
                capture_output=True, text=True, check=True
            ).stdout.strip()
            
            libs_dir = os.path.join(artifact_dir, 'libs', tuple_name)
            os.makedirs(libs_dir, exist_ok=True)
            
            print(f"Tuple name: {tuple_name}")
            
            # Get required libraries from the binary
            target_binary = os.path.join(artifact_dir, app_config["target"])
            result = subprocess.run(
                ['docker', 'run', '--rm', '-v', f'{artifact_dir}:/artifacts', artifact_image, 
                 'readelf', '-d', f'/artifacts/{app_config["target"]}'],
                capture_output=True, text=True, check=True
            )
            
            required_libs = set()
            for line in result.stdout.split('\n'):
                if 'NEEDED' in line:
                    lib = line.split('[')[1].split(']')[0]
                    required_libs.add(lib)
            
            print(f"Required libraries: {', '.join(sorted(required_libs))}")
            
            # Copy only required libraries and their symlinks
            sysroot_lib = f'/home/builder/x-tools/{tuple_name}/{tuple_name}/sysroot/lib'
            for lib in required_libs:
                # Copy the library and any symlinks
                result = subprocess.run(
                    ['docker', 'run', '--rm', artifact_image, 'find', sysroot_lib, '-name', f'{lib}*'],
                    capture_output=True, text=True, check=True
                )
                for lib_path in result.stdout.strip().split('\n'):
                    if lib_path:
                        lib_name = os.path.basename(lib_path)
                        subprocess.run(
                            ['docker', 'cp', f'{container_id}:{lib_path}', os.path.join(libs_dir, lib_name)],
                            check=True
                        )
            
            # Always copy the dynamic linker
            result = subprocess.run(
                ['docker', 'run', '--rm', artifact_image, 'find', sysroot_lib, '-name', 'ld-linux*.so*'],
                capture_output=True, text=True, check=True
            )
            for ld_path in result.stdout.strip().split('\n'):
                if ld_path:
                    ld_name = os.path.basename(ld_path)
                    subprocess.run(
                        ['docker', 'cp', f'{container_id}:{ld_path}', os.path.join(libs_dir, ld_name)],
                        check=True
                    )
            
            # Remove non-runtime files
            for root, dirs, files in os.walk(libs_dir):
                for f in files:
                    if f.endswith(('.a', '.py', '.json')):
                        os.remove(os.path.join(root, f))
            
            subprocess.run(['chmod', '-R', 'u+rwX', libs_dir], check=False)
            
            print("\nLibrary files copied:")
            total_size = 0
            for f in sorted(os.listdir(libs_dir)):
                fpath = os.path.join(libs_dir, f)
                if os.path.isfile(fpath):
                    size = os.path.getsize(fpath)
                    total_size += size
                    print(f"  {f:40s} {size:>10,} bytes")
            print(f"\nTotal library size: {total_size:,} bytes ({total_size / 1024 / 1024:.2f} MB)")
            lib_size = total_size
        
        subprocess.run(['docker', 'rm', container_id], check=True)
        
        runtime_base = f's3lc-runtime:{platform}'
        runtime_dockerfile = platform_config.get('runtime', 'amd64/runtime')
        cmd_runtime = ['docker', 'build']
        if NETWORK_HOST:
            cmd_runtime.extend(['--network=host'])
        cmd_runtime.extend(['--platform', platform_config['platform']])
        if tuple_name:
            cmd_runtime.extend(['--build-arg', f'TUPLE={tuple_name}'])
        cmd_runtime.extend([
            '-f', os.path.join(SCRIPT_DIR, runtime_dockerfile),
            '-t', runtime_base,
            artifact_dir
        ])
        
        if not run_cmd(cmd_runtime):
            return None
        
        if registry:
            image_name = f'{registry}/supl-3gpp-lpp-client/{app}/{platform}:{tag}'
        else:
            image_name = f's3lc/{app}/{platform}:{tag}'
        
        cmd_final = ['docker', 'build']
        if NETWORK_HOST:
            cmd_final.extend(['--network=host'])
        cmd_final.extend(['--platform', platform_config['platform']])
        cmd_final.extend([
            '--build-arg', f'RUNTIME_BASE={runtime_base}',
            '--build-arg', f'TARGET={app_config["target"]}',
            '--label', 'org.opencontainers.image.source=https://github.com/Ericsson/SUPL-3GPP-LPP-client',
            '-f', os.path.join(SCRIPT_DIR, 'Dockerfile.target'),
            '-t', image_name,
            artifact_dir
        ])
        
        if run_cmd(cmd_final):
            return (image_name, lib_size)
        return None
    finally:
        shutil.rmtree(artifact_dir, ignore_errors=True)

def merge_multiarch(app, platforms, built_images, registry, tag):
    """Merge platform-specific images into a multi-arch manifest using imagetools"""
    print(f"\n{colorize('===', Colors.CYAN)} {colorize(f'Merging multi-arch {app}', Colors.BOLD)} {colorize('===', Colors.CYAN)}")
    
    # Find images for this app
    app_images = [img for img in built_images if f'/{app}:' in img or f'-{app}:' in img]
    if not app_images:
        print(colorize(f"No images found for {app}", Colors.RED))
        return False
    
    multiarch_name = f'{registry}/supl-3gpp-lpp-client/{app}:{tag}'
    
    cmd = ['docker', 'buildx', 'imagetools', 'create', '-t', multiarch_name] + app_images
    return run_cmd(cmd)

def push_image(image_name, force=False):
    """Push a single image"""
    # Check for dirty tag
    if '-dirty' in image_name and not force:
        print(colorize(f"ERROR: Cannot push dirty tag: {image_name}", Colors.RED))
        print("Commit your changes before pushing (or use --force)")
        return False
    
    return run_cmd(['docker', 'push', image_name])

def get_image_breakdown(image_name):
    """Get breakdown of image components"""
    # Get total size in bytes
    result = subprocess.run(
        ['docker', 'inspect', '--format', '{{.Size}}', image_name],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        return None
    
    total_bytes = int(result.stdout.strip())
    
    # Get layer history
    result = subprocess.run(
        ['docker', 'history', '--no-trunc', '--format', '{{.Size}}\t{{.CreatedBy}}', image_name],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        return None
    
    os_size = 0
    libs_size = 0
    exec_size = 0
    
    for line in result.stdout.strip().split('\n'):
        parts = line.split('\t', 1)
        if len(parts) != 2:
            continue
        size_str, cmd = parts
        
        # Parse size (e.g., "10.5MB", "1.2kB", "0B")
        if size_str == '0B' or size_str == '0':
            continue
        
        size_bytes = 0
        if 'MB' in size_str:
            size_bytes = int(float(size_str.replace('MB', '')) * 1024 * 1024)
        elif 'kB' in size_str:
            size_bytes = int(float(size_str.replace('kB', '')) * 1024)
        elif 'GB' in size_str:
            size_bytes = int(float(size_str.replace('GB', '')) * 1024 * 1024 * 1024)
        elif 'B' in size_str:
            size_bytes = int(size_str.replace('B', ''))
        
        # Categorize based on command
        if 'apt-get' in cmd or 'FROM' in cmd:
            os_size += size_bytes
        elif 'COPY' in cmd and 'example-' in cmd:
            exec_size += size_bytes
        elif 'apt-get install' in cmd:
            libs_size += size_bytes
        else:
            os_size += size_bytes
    
    return {
        'total': total_bytes,
        'os': os_size,
        'libs': libs_size,
        'exec': exec_size
    }

def format_bytes(bytes_val):
    """Format bytes to human readable"""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if bytes_val < 1024:
            return f"{bytes_val:.1f}{unit}"
        bytes_val /= 1024
    return f"{bytes_val:.1f}TB"

def print_build_summary(built_images):
    """Print summary of built images before pushing"""
    print(f"\n{colorize('='*120, Colors.BOLD)}")
    print(colorize('BUILD SUMMARY', Colors.BOLD))
    print(colorize('='*120, Colors.BOLD))
    
    for img in built_images:
        image_name = img['name'] if isinstance(img, dict) else img
        size = get_image_info(image_name)
        print(f"  {colorize('✓', Colors.GREEN)} {image_name:<110} {colorize(size, Colors.CYAN)}")
    
    print(f"\n{colorize(f'Total: {len(built_images)} images built', Colors.BOLD)}")
    
    # Print breakdown for all images
    print(f"\n{colorize('='*120, Colors.BOLD)}")
    print(colorize('IMAGE BREAKDOWN', Colors.BOLD))
    print(colorize('='*120, Colors.BOLD))
    
    total_runtime = 0
    total_exec = 0
    total_libs = 0
    total_all = 0
    
    # Print header
    print(f"\n{'Image':<100} {'Runtime':>10} {'Libraries':>10} {'Executable':>10} {'Total':>10}")
    print(f"{'-'*80} {'-'*10} {'-'*10} {'-'*10} {'-'*10}")
    
    for img in built_images:
        image_name = img['name'] if isinstance(img, dict) else img
        lib_size = img.get('lib_size', 0) if isinstance(img, dict) else 0
        
        breakdown = get_image_breakdown(image_name)
        if not breakdown:
            continue
        
        total = breakdown['total']
        runtime_size = breakdown['os']
        exec_size = breakdown['exec']
        
        total_runtime += runtime_size
        total_exec += exec_size
        total_libs += lib_size
        total_all += total
        
        print(f"{image_name:<100} {format_bytes(runtime_size):>10} {format_bytes(lib_size):>10} {format_bytes(exec_size):>10} {format_bytes(total):>10}")
    
    # Print totals
    if total_all > 0:
        print(f"{'-'*80} {'-'*10} {'-'*10} {'-'*10} {'-'*10}")
        runtime_pct = (total_runtime / total_all * 100)
        libs_pct = (total_libs / total_all * 100)
        exec_pct = (total_exec / total_all * 100)
        print(f"{'TOTAL':<100} {format_bytes(total_runtime):>10} {format_bytes(total_libs):>10} {format_bytes(total_exec):>10} {format_bytes(total_all):>10}")

def print_push_summary(pushed_images):
    """Print summary of pushed images"""
    print(f"\n{colorize('='*80, Colors.BOLD)}")
    print(colorize('PUSH SUMMARY', Colors.BOLD))
    print(colorize('='*80, Colors.BOLD))
    
    for image_name in pushed_images:
        print(f"  {colorize('✓', Colors.GREEN)} {colorize(image_name, Colors.CYAN)}")
    
    print(f"\n{colorize(f'Total: {len(pushed_images)} images pushed', Colors.BOLD)}")

def validate_platforms():
    """Validate PLATFORMS configuration"""
    for platform, config in PLATFORMS.items():
        if 'builder' in config and 'cross' in config:
            print(colorize(f"ERROR: Platform '{platform}' has both 'builder' and 'cross' defined", Colors.RED))
            print("Use 'builder' for native compilation OR 'cross' for cross-compilation, not both")
            sys.exit(1)

def main():
    global NETWORK_HOST
    
    validate_platforms()
    
    parser = argparse.ArgumentParser(description='Build and push multi-platform Docker images')
    parser.add_argument('--platform', '-p', action='append', choices=list(PLATFORMS.keys()) + ['all'],
                        help='Platform to build (can specify multiple, default: all)')
    parser.add_argument('--app', '-a', action='append', choices=list(APPS.keys()) + ['all'],
                        help='Application to build (can specify multiple, default: all)')
    parser.add_argument('--build-mode', '-m', action='append', choices=BUILD_MODES + ['all'],
                        help='Build mode (can specify multiple, default: release)')
    parser.add_argument('--push', action='store_true', help='Push images after building')
    parser.add_argument('--dry-run', action='store_true', help='Show what would be pushed without actually pushing')
    parser.add_argument('--registry', '-r', default='ghcr.io/ericsson',
                        help='Container registry (default: ghcr.io/ericsson)')
    parser.add_argument('--tag', '-t', help='Image tag (default: auto-generated)')
    parser.add_argument('--latest', action='store_true', help='Also push as latest (only works with version tags)')
    parser.add_argument('--wip', action='store_true', help='Push as wip tag (cannot be used with version tags or --latest)')
    parser.add_argument('--force', action='store_true', help='Force push even if dirty check fails')
    parser.add_argument('--multiarch', action='store_true', help='Build multi-arch images using buildx')
    parser.add_argument('--network-host', action='store_true', help='Use --network=host for all docker commands')
    parser.add_argument('--clean', action='store_true', help='Clean build (remove build cache)')
    args = parser.parse_args()
    
    NETWORK_HOST = args.network_host
    
    # Validate --latest flag
    if args.latest and (not args.tag or not args.tag.startswith('v')):
        print(colorize("ERROR: --latest can only be used with version tags (e.g., v1.0.0)", Colors.RED))
        sys.exit(1)
    
    # Validate --wip flag
    if args.wip and (args.tag or args.latest):
        print(colorize("ERROR: --wip cannot be used with --tag or --latest", Colors.RED))
        sys.exit(1)
    
    # Parse platforms
    if args.platform:
        platforms = []
        for p in args.platform:
            if p == 'all':
                platforms = list(PLATFORMS.keys())
                break
            platforms.append(p)
    else:
        platforms = list(PLATFORMS.keys())
    
    # Parse apps
    if args.app:
        apps = []
        for a in args.app:
            if a == 'all':
                apps = list(APPS.keys())
                break
            apps.append(a)
    else:
        apps = list(APPS.keys())
    
    # Parse build modes
    if args.build_mode:
        build_modes = []
        for m in args.build_mode:
            if m == 'all':
                build_modes = BUILD_MODES
                break
            build_modes.append(m)
    else:
        build_modes = ['release']
    
    built_images = []
    built_artifacts = set()
    
    # Clean build cache if requested (but keep crosstool builders)
    if args.clean:
        print(f"\n{colorize('===', Colors.CYAN)} {colorize('Cleaning build cache (keeping crosstool)', Colors.BOLD)} {colorize('===', Colors.CYAN)}")
        for build_mode in build_modes:
            for platform in platforms:
                # Remove artifact cache images
                cache_id = f's3lc-cache-{build_mode}-{platform}'
                result = subprocess.run(['docker', 'images', '-q', cache_id], capture_output=True, text=True)
                if result.stdout.strip():
                    print(f"  Removing: {cache_id}")
                    subprocess.run(['docker', 'rmi', '-f', cache_id], check=False)
                
                # Remove artifact images
                for app in apps:
                    artifact_pattern = f's3lc-artifact:*-{platform}'
                    result = subprocess.run(['docker', 'images', '-q', artifact_pattern], capture_output=True, text=True)
                    for img_id in result.stdout.strip().split('\n'):
                        if img_id:
                            subprocess.run(['docker', 'rmi', '-f', img_id], check=False)
        print(f"  {colorize('✓', Colors.GREEN)} Build cache cleaned (crosstool builders preserved)")
    
    for platform in platforms:
        if not build_builder_image(platform):
            print(colorize(f"Failed to build builder image for {platform}", Colors.RED))
            sys.exit(1)
    
    for build_mode in build_modes:
        for app in apps:
            for platform in platforms:
                tag_to_use = args.tag if args.tag else None
                result = build_image(app, platform, build_mode, args.registry if (args.push or args.dry_run) else None, tag_to_use, built_artifacts)
                if not result:
                    print(colorize(f"Failed to build {app} for {platform} ({build_mode})", Colors.RED))
                    sys.exit(1)
                
                image_name, lib_size = result
                tag_suffix = 'latest' if args.latest else ('wip' if args.wip else None)
                built_images.append({
                    'name': image_name,
                    'app': app,
                    'platform': platform,
                    'build_mode': build_mode,
                    'tag_suffix': tag_suffix,
                    'lib_size': lib_size
                })
    
    print(f"\n{colorize('All builds completed successfully!', Colors.GREEN)}")
    print_build_summary(built_images)
    
    if args.push or args.dry_run:
        push_list = []
        for img in built_images:
            # Only push clean commits (no -dirty), skip dirty unless it's wip/latest
            if '-dirty' not in img['name'] or img['tag_suffix']:
                if img['tag_suffix']:
                    # For wip/latest, create clean tag
                    parts = img['name'].rsplit(':', 1)
                    base = parts[0]
                    tag = parts[1]
                    
                    # Create wip/latest tag without dirty
                    if '-debug' in tag:
                        new_tag = f"{img['tag_suffix']}-debug"
                    else:
                        new_tag = img['tag_suffix']
                    
                    new_name = f'{base}:{new_tag}'
                    push_list.append((img['name'], new_name))
                else:
                    # Push clean commit tags as-is
                    push_list.append((img['name'], img['name']))
        
        print(f"\n{colorize('='*80, Colors.BOLD)}")
        if args.dry_run:
            print(colorize('DRY RUN - WOULD PUSH', Colors.BOLD))
        else:
            print(colorize('PUSHING IMAGES', Colors.BOLD))
        print(colorize('='*80, Colors.BOLD))
        
        pushed_images = []
        for source, target in push_list:
            if source != target and not args.dry_run:
                run_cmd(['docker', 'tag', source, target])
            
            if args.dry_run:
                print(f"  {colorize('○', Colors.YELLOW)} {target}")
            else:
                if not push_image(target, force=args.force or args.wip):
                    print(f"  {colorize('✗', Colors.RED)} {target}")
                    sys.exit(1)
                print(f"  {colorize('✓', Colors.GREEN)} {target}")
            pushed_images.append(target)
        
        if args.multiarch and not args.dry_run:
            print(f"\n{colorize('='*80, Colors.BOLD)}")
            print(colorize('CREATING MULTIARCH MANIFESTS', Colors.BOLD))
            print(colorize('='*80, Colors.BOLD))
            
            for app in apps:
                for build_mode in build_modes:
                    tag_suffix = 'latest' if args.latest else 'wip'
                    version_tag = f'{tag_suffix}-debug' if build_mode == 'debug' else tag_suffix
                    
                    # Find all platform-specific images for this app/mode
                    app_images = [img for img in pushed_images 
                                 if f'/{app}/' in img and img.endswith(f':{version_tag}')]
                    
                    if app_images:
                        if args.registry:
                            manifest_name = f'{args.registry}/supl-3gpp-lpp-client/{app}:{version_tag}'
                        else:
                            manifest_name = f's3lc/{app}:{version_tag}'
                        cmd = ['docker', 'buildx', 'imagetools', 'create', '-t', manifest_name] + app_images
                        run_cmd(cmd)
                        print(f"  {colorize('✓', Colors.GREEN)} {manifest_name}")
        
        print(f"\n{colorize(f'Total: {len(pushed_images)} images {"would be pushed" if args.dry_run else "pushed"}', Colors.BOLD)}")

if __name__ == '__main__':
    main()
