# Example - LPP Client

This sample code is a simple client that asks for assistance data from a location server. It can handle OSR, SSR, and AGNSS requests. The assistance data can converted to RTCM or SPARTN before being sent to a GNSS receiver or other interface. The client also supports to 3GPP LPP Provide Location Information, which can be used to send the device's location to the location server. 

## Usage

There are a few required arguments:
- `-h` or `--host` - The host of the location server
- `-c` or `--mcc` - The Mobile Country Code
- `-n` or `--mnc` - The Mobile Network Code
- `-t` or `--tac` - The Tracking Area Code
- `-i` or `--ci` - The Cell Identity

```
  ./example-lpp COMMAND {OPTIONS}

    3GPP LPP Example (3.4.4) - This sample code is a simple client that asks for
    assistance data from a location server. It can handle OSR, SSR, and AGNSS
    requests. The assistance data can converted to RTCM or SPARTN before being
    sent to a GNSS receiver or other interface. The client also supports to 3GPP
    LPP Provide Location Information, which can be used to send the device's
    location to the location server.

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
        --supl-identity-fix               Use SUPL Identity Fix
      Cell Information:
        -c[mcc], --mcc=[mcc]              Mobile Country Code
        -n[mnc], --mnc=[mnc]              Mobile Network Code
        -t[tac], --lac=[tac], --tac=[tac] Tracking Area Code
        -i[ci], --ci=[ci]                 Cell Identity
        --nr                              The cell specified is a 5G NR cell
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
      NMEA Receiver:
        Serial:
        --nmea-serial=[device]            Device
        --nmea-serial-baud=[baud_rate]    Baud Rate
                                          Default: 115200
        --nmea-serial-data=[data_bits]    Data Bits
                                          One of: 5, 6, 7, 8
                                          Default: 8
        --nmea-serial-stop=[stop_bits]    Stop Bits
                                          One of: 1, 2
                                          Default: 1
        --nmea-serial-parity=[parity_bits]
                                          Parity Bits
                                          One of: none, odd, even
                                          Default: none
        --nmea-export-un=[unix socket]    Export NMEA to unix socket
        --nmea-export-tcp=[ip]            Export NMEA to TCP
        --nmea-export-tcp-port=[port]     Export NMEA to TCP Port
      Other Receiver Options:
        --print-receiver-messages, --prm  Print Receiver Messages
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
      Location Infomation:
        --fake-location-info, --fli       Enable sending fake location
                                          information. Configure with '--fake-*'
                                          options.
        --force-location-info             Force Location Information (always
                                          send even if not requested)
        --location-report-unlocked        Send location reports without locking
                                          the update rate. By default, the
                                          update rate is locked to 1 second.
        --fake-latitude=[latitude],
        --flat=[latitude]                 Fake Latitude
                                          Default: 69.0599730655754
        --fake-longitude=[longitude],
        --flon=[longitude]                Fake Longitude
                                          Default: 20.54864403253676
        --fake-altitude=[altitude],
        --falt=[altitude]                 Fake Altitude
                                          Default: 0
```
