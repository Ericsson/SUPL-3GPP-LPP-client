# NTRIP Example

This sample code illustrates the process of utilizing the NTRIP client to establish a communication link with a caster and transmit the data to a serial port, file, or stdout.

## Usage

```
  ./example-ntrip {OPTIONS}

    NTRIP Example (3.4.0) - This sample code illustrates the process of
    utilizing the NTRIP client to establish a communication link with a caster
    and transmit the data to a serial port, file, or stdout.

  OPTIONS:

      -?, --help                        Display this help menu
      -v, --version                     Display version information
      NTRIP:
        --host=[hostname]                 Hostname
        --port=[port]                     Port
                                          Default: 2101
        --mountpoint=[mountpoint]         Mountpoint
        --username=[username]             Username
        --password=[password]             Password
        --nmea=[nmea_string]              NMEA String
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
