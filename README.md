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

The example client requires that you provide connection parameters of a location server and the ECGI (including TAC) that should be used. There are additional options to configure RTCM MSM type generation and where the final RTCM messages you be transported.

> Version +3.1.0 has a new argument parser. New options may be required and short-hand notation for many of them has changed. Now the example client is divided into 3 parts (osr, ssr, and agnss), use a command to specifiy which part you want to run. For more information see run `./example`

```
  ./src/example COMMAND {OPTIONS}

    SUPL-3GPP-LPP-client v3.2.1 (public)

  OPTIONS:

      -?, --help                        Display this help menu
      -v, --version                     Display version information
      Commands:
        osr                               Request Observation Space
                                          Representation (OSR) data from the
                                          location server.
        ssr                               Request State-space Representation
                                          (SSR) data from the location server.
        agnss                             Request Assisted GNSS data from the
                                          location server.
      Location Server:
        -h[host], --host=[host]           Host
        -p[port], --port=[port]           Port
                                          Default: 5431
        -s, --ssl                         TLS
                                          Default: false
      Identity:
        --msisdn=[msisdn]                 MSISDN
        --imsi=[imsi]                     IMSI
        --ipv4=[ipv4]                     IPv4
      Cell Information:
        -c[mcc], --mcc=[mcc]              Mobile Country Code
        -n[mnc], --mnc=[mnc]              Mobile Network Code
        -t[tac], --lac=[tac], --tac=[tac] Tracking Area Code
        -i[ci], --ci=[ci]                 Cell Identity
      Modem:
        --modem=[device]                  Device
        --modem-baud=[baud_rate]          Baud Rate
      Output:
        File:
          --file=[file_path]                Path
        Serial:
          --serial=[device]                 Device
          --serial-baud=[baud_rate]         Baud Rate
                                            Default: 115200
        I2C:
          --i2c=[device]                    Device
          --i2c-address=[address]           Address
        TCP:
          --tcp=[ip_address]                IP Address
          --tcp-port=[port]                 Port
        UDP:
          --udp=[ip_address]                IP Address
          --udp-port=[port]                 Port
        Stdout:
          --stdout                          Stdout
```

## License
See [LICENSE](/LICENSE.txt) file.
