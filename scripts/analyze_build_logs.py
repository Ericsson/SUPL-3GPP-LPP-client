#!/usr/bin/env python3
import re
import sys
from pathlib import Path
from collections import defaultdict

RED = "\033[91m"
YELLOW = "\033[93m"
GREEN = "\033[92m"
BLUE = "\033[94m"
RESET = "\033[0m"

build_dir = Path.home() / ".cache" / "SUPL-3GPP-LPP-client" / "tests"
pattern = re.compile(r"(warning|error):\s*(.+)", re.IGNORECASE)

warnings = defaultdict(set)
errors = defaultdict(set)

print(f"{BLUE}Finding build.log files...{RESET}")
log_files = list(build_dir.rglob("build.log"))
print(f"{GREEN}Found {len(log_files)} files{RESET}\n")

for i, log_file in enumerate(log_files, 1):
    config = log_file.parent.name.split("-")
    config_key = tuple(config)
    
    sys.stdout.write(f"\r{BLUE}Scanning [{i}/{len(log_files)}]{RESET} {log_file.parent.name[:50]:<50}")
    sys.stdout.flush()
    
    with open(log_file, "r", errors="ignore") as f:
        for line in f:
            match = pattern.search(line)
            if match:
                msg_type, msg = match.groups()
                if msg_type.lower() == "warning":
                    warnings[msg.strip()].add(config_key)
                else:
                    errors[msg.strip()].add(config_key)

print(f"\n\n{RED}=== ERRORS ==={RESET}")
for error in sorted(errors.keys(), key=lambda x: len(errors[x]), reverse=True):
    configs = sorted(errors[error])
    first = "-".join(configs[0])
    extra = f" +{len(configs)-1} more" if len(configs) > 1 else ""
    print(f"  {error} {BLUE}[{first}{extra}]{RESET}")

print(f"\n{YELLOW}=== WARNINGS ==={RESET}")
for warning in sorted(warnings.keys(), key=lambda x: len(warnings[x]), reverse=True):
    configs = sorted(warnings[warning])
    first = "-".join(configs[0])
    extra = f" +{len(configs)-1} more" if len(configs) > 1 else ""
    print(f"  {warning} {BLUE}[{first}{extra}]{RESET}")

print(f"\n{GREEN}Total: {len(errors)} unique errors, {len(warnings)} unique warnings{RESET}")
