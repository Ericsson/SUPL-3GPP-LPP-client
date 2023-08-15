# Example - LPP Client

This is a simple client examples that requests assistance data from a location server. It supports OSR, SSR, and AGNSS requests. The assistance data can be outputted to a file, serial port, TCP, UDP or stdout. It can also be converted to RTCM messages that can be transmitted to any GNSS receiver that supports it.

## Usage

There are a few required arguments:
- `-h` or `--host` - The host of the location server
- `-c` or `--mcc` - The Mobile Country Code
- `-n` or `--mnc` - The Mobile Network Code
- `-t` or `--tac` - The Tracking Area Code
- `-i` or `--ci` - The Cell Identity

```
./examples/lpp/example_lpp COMMAND {OPTIONS}

Example - LPP v3.3.0 (public)

OPTIONS:

    -?, --help                        Display this help menu
    -v, --version                     Display version information
    Commands:
    osr                               Request observation data from a
                                        location server
    ssr                               Request State-space Representation
                                        (SSR) data from the location server
    agnss                             Request Assisted GNSS data from the
                                        location server
    Location Server:
    -h[host], --host=[host]           Host
    -p[port], --port=[port]           Port
                                        Default: 5431
    -s, --ssl                         TLS
                                        Default: false
    Identity:
    --msisdn=[msisdn]                 MSISDN
    --imsi=[imsi]                     IMSI
                                        Default: 2460813579
    --ipv4=[ipv4]                     IPv4
    Cell Information:
    -c[mcc], --mcc=[mcc]              Mobile Country Code
    -n[mnc], --mnc=[mnc]              Mobile Network Code
    -t[tac], --lac=[tac], --tac=[tac] Tracking Area Code
    -i[ci], --ci=[ci]                 Cell Identity
    Modem:
    --modem=[device]                  Device
    --modem-baud=[baud_rate]          Baud Rate
    u-blox Receiver:
    --ublox-port=[port]               The port used on the u-blox receiver,
                                        used by configuration.
                                        One of: uart1, uart2, i2c, usb
                                        Default: (by interface)
    Serial:
        --ublox-serial=[device]           Device
        --ublox-serial-baud=[baud_rate]   Baud Rate
                                        Default: 115200
        --ublox-serial-data=[data_bits]   Data Bits
                                        One of: 5, 6, 7, 8
                                        Default: 8
        --ublox-serial-stop=[stop_bits]   Stop Bits
                                        One of: 1, 2
                                        Default: 1
        --ublox-serial-parity=[parity_bits]
                                        Parity Bits
                                        One of: none, odd, even
                                        Default: none
    I2C:
        --ublox-i2c=[device]              Device
        --ublox-i2c-address=[address]     Address
                                        Default: 66
    TCP:
        --ublox-tcp=[ip_address]          Host or IP Address
        --ublox-tcp-port=[port]           Port
    UDP:
        --ublox-udp=[ip_address]          Host or IP Address
        --ublox-udp-port=[port]           Port
    Output:
    File:
        --file=[file_path]                Path
    Serial:
        --serial=[device]                 Device
        --serial-baud=[baud_rate]         Baud Rate
                                        Default: 115200
        --serial-data=[data_bits]         Data Bits
                                        One of: 5, 6, 7, 8
                                        Default: 8
        --serial-stop=[stop_bits]         Stop Bits
                                        One of: 1, 2
                                        Default: 1
        --serial-parity=[parity_bits]     Parity Bits
                                        One of: none, odd, even
                                        Default: none
    I2C:
        --i2c=[device]                    Device
        --i2c-address=[address]           Address
                                        Default: 66
    TCP:
        --tcp=[ip_address]                Host or IP Address
        --tcp-port=[port]                 Port
    UDP:
        --udp=[ip_address]                Host or IP Address
        --udp-port=[port]                 Port
    Stdout:
        --stdout                          Stdout
```
