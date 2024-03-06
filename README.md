# SUPL 3GPP LPP client

![version](https://img.shields.io/badge/version-3.4.4-green)
![license](https://img.shields.io/badge/license-MXM-blue)

This project is a set of libraries, examples and tools to facilitate the development of 3GPP LPP clients. 

## Libraries
* 3GPP LPP client - A library that can be used to communicate with a SUPL server and request assistance data.
* RTCM converter - Converts RTCM messages to 3GPP LPP messages
* SPARTN converter - Converts 3GPP LPP messages to SPARTN messages
* Interface - A set of interfaces that can be read or written to, e.g. a device, a file, a socket, etc.
* u-Blox Receiver - A library that can be used to communicate with a u-Blox receiver.
* NMEA Receiver - A library that can be used to communicate with a GNSS receiver that supports NMEA.

## Examples
* [3GPP LPP example](/examples/lpp/README.md) - `example-lpp` - Simple example of requesting assistance data from a SUPL server, converting it, and sending it to a GNSS receiver. Supports OSR, SSR, and basic retrieval of A-GNSS data. This is almost a fully implemented client.
* [NTRIP example](/examples/ntrip/README.md) - `example-ntrip` - Example that connects to an NTRIP caster, requesting RTCM data, and sending it to a GNSS receiver.
* [u-Blox example](/examples/ublox/README.md) - `example-ublox` - Example that will communicate with a u-Blox receiver and print all received messages to the console.
* [NMEA example](/examples/nmea/README.md) - `example-nmea` - Example that will communicate with a GNSS receiver (that supports NMEA) and print all received messages to the console.

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
