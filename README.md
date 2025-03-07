# SUPL 3GPP LPP client

![version](https://img.shields.io/badge/version-4.0.1-green)
![license](https://img.shields.io/badge/license-MXM-blue)

This project is a set of libraries, examples and tools to facilitate the development of 3GPP LPP clients. 

> [!IMPORTANT]
> Upgrading from version 3.4 to version 4 have breaking changes. Follow [Upgrade Guide](/UPGRADE_FROM_V3.md) for argument changes when using `example-lpp`.  

## Libraries
* 3GPP LPP client - A library that can be used to communicate with a SUPL server and request assistance data.
* LPP to RTCM converter - Convert 3GPP LPP messages to RTCM messages
* LPP to SPARTN converter - Convert 3GPP LPP messages to SPARTN messages
* LPP SSR to OSR RTCM converter - Convert 3GPP LPP SSR messages to OSR RTCM messages

## Examples
* [Client example](/examples/client/README.md) - `example-client` - Example client that integrates all library functionality into one usable client. 
* [NTRIP example](/examples/ntrip/README.md) - `example-ntrip` - Example that connects to an NTRIP caster, requesting RTCM data, and sending it to a GNSS receiver.

## Build

First install the dependencies:
```bash
sudo apt install g++ cmake libssl-dev ninja-build
```

Clone the repository:
```bash
git clone git@github.com:Ericsson/SUPL-3GPP-LPP-client.git
cd SUPL-3GPP-LPP-client
```

Setup the build:
```bash
mkdir build
cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug
```

Build the project (from the build directory):
```bash
ninja
```

## License
See [LICENSE](/LICENSE.txt) file.
