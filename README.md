# SUPL-3GPP-LPP-client

SUPL-3GPP-LPP-client is an example client for connecting requesting assistance data from a location server using 3GPP LPP (Release 16.4) over SUPL. The client includes a handful of libraries that can be used to build a more feature rich and fully functioning client, that for example communications with a GNSS receiver to estimate a precise position. 

The default configuration of the example client is to connect to the location server using the provided ECGI and request assistance data (for OSR). Data received will be converted to RTCM that can be used with legacy devices. Requesting SSR corrections is supported but requires defining SSR=1 when building, the RTCM converter cannot be used with SSR data.

The example client uses RTKLIB (https://github.com/tomojitakasu/RTKLIB) to help with encoding RTCM message and generated source code from ASN.1 C (https://github.com/vlm/asn1c) for encoding and decoding 3GPP LPP Release 16.4 with UPER. All other source code is written for the project in C/C++ style that should compile with most compilers with C++11 support.

---

## Table of Contents
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Usage](#usage)
- [License](#license)

---
 
## Prerequisites
These are the prerequisites required for building: 
```bash
sudo apt install g++ cmake libssl-dev ninja-build
```

# Building
Building using CMake and Ninja:
```bash
cmake -S . -B build -G Ninja
cmake --build build --config Debug
```

After a successful build, the example client executable should be available at `build/src/example`.

### GCC-4.8

To verify compatibility with gcc-4.8 and older compilers, use the build script (`build-with-gcc4.8.sh`), that will build the client with gcc 4.8. This help guarantee that the source code will compile on smaller devices such as a Raspberry PI or BeagleBone. This is done with the help of a docker image with the compiler preinstalled. (This requires that you have docker installed)

## Usage

The example client requires that you provide connection parameters of a location server and the ECGI (including TAC) that should be used. There are additional options to configure RTCM MSM type generation and where the final RTCM messages you be transported. Support for outputting RTCM messages to device is supported but command-line options are not displayed in the help prompt.

```
./example -?

options:
-?, --help              show help prompt
-h, --host              location server host [host]
-p, --port              location server post [integer]     default=5431
-s, --ssl               location server ssl  [flag]        default=off
-c, --mcc               mobile country code  [integer]
-n, --mnc               mobile network code  [integer]
-i, --cellid            cell id              [integer]
-t, --tac               tracking area code   [integer]
-y, --msm_type          msm type             [integer]     default=-1 (best suitable)
-k, --server_ip         server host          [host]        default="" (no output server)
-o, --server_port       server port          [integer]     default=3000
-x, --file_output       file path output     [path]        default="" (no output file)
    --modem_device      modem path           [path]        default="" (no modem connected)
    --modem_baud_rate   modem baud rate      [integer]     default=9600
```

## License
See [LICENSE](/LICENSE.txt) file.

