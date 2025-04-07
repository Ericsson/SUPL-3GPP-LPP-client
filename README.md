# SUPL 3GPP LPP client

![version](https://img.shields.io/badge/version-4.0.11-green)
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

## License
See [LICENSE](/LICENSE.txt) file.
