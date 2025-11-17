# SUPL 3GPP LPP client

![version](https://img.shields.io/badge/version-4.0.22-green)
![license](https://img.shields.io/badge/license-MXM-blue)

A comprehensive toolkit for developing 3GPP LPP (LTE Positioning Protocol) clients. This project provides libraries and tools for handling SUPL (Secure User Plane Location) communication and various positioning-related message conversions.

> [!IMPORTANT]  
> Version 4.0 introduces breaking changes from version 3.4. Please consult the [Upgrade Guide](/UPGRADE_FROM_V3.md) before updating, particularly if you're using `example-lpp`.

## Features
### Core Libraries
- **3GPP LPP Client Library**: Enables SUPL server communication and assistance data retrieval
- **Message Converters**:
  - LPP to RTCM: Converts 3GPP LPP messages to RTCM format
  - LPP to SPARTN: Transforms 3GPP LPP messages to SPARTN format
  - LPP SSR to OSR RTCM: Handles conversion from 3GPP LPP SSR to OSR RTCM messages

### Example Applications
- **Client Demo** (`example-client`): Demonstrates full integration of library functionality [Documentation](/examples/client/README.md)
- **NTRIP Demo** (`example-ntrip`): NTRIP client with integrated GNSS receiver communication [Documentation](/examples/ntrip/README.md)

## Getting Started

### Prerequisites
The following dependencies are required:
```bash
sudo apt install g++ cmake libssl-dev ninja-build
```

### Installation
Clone the repository:
```bash
git clone git@github.com:Ericsson/SUPL-3GPP-LPP-client.git
cd SUPL-3GPP-LPP-client
```

### Build
Configure the build:
```bash
mkdir build
cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug
```

Build the project:
```bash
ninja
```

### Testing
Enable and run tests:
```bash
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
ninja
ctest --output-on-failure
```

### Fuzzing
Enable and run fuzzing (requires Clang with libFuzzer):
```bash
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DENABLE_FUZZING=ON
ninja fuzz_nmea fuzz_ubx fuzz_rtcm fuzz_at
./tests/fuzz_nmea ../tests/corpus/nmea -max_total_time=60
./tests/fuzz_ubx ../tests/corpus/ubx -max_total_time=60
./tests/fuzz_rtcm ../tests/corpus/rtcm -max_total_time=60
./tests/fuzz_at ../tests/corpus/at -max_total_time=60
```

The corpus directory contains seed inputs for better fuzzing coverage. New interesting inputs discovered during fuzzing are automatically added to the corpus.

### Static Analysis (Optional)
Enable static analyzers during build:
```bash
cmake .. -GNinja -DENABLE_CLANG_TIDY=ON
cmake .. -GNinja -DENABLE_CPPCHECK=ON
cmake .. -GNinja -DENABLE_IWYU=ON
```

## License
See [LICENSE](/LICENSE.txt) file.
