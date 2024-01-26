# LPP2SPARTN Example

This sample code takes UPER encoded 3GPP LPP messages from STDIN and transforms them into SPARTN messages.

## Usage

```
  ./example-lpp2spartn {OPTIONS}

    LPP2SPARTN Example (3.4.0) - This sample code takes UPER encoded 3GPP LPP
    messages from STDIN and transforms them into SPARTN messages.

  OPTIONS:

      -?, --help                        Display this help menu
      -v, --version                     Display version information
      Options:
        --format=[format]                 Format
                                          One of: spartn, spartn-old
        --iode-shift                      IODE Shift
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
