# NMEA Example 

This is an example program that demonstrates how to use the NMEA receiver library. The program takes an interface and port associated with the receiver as arguments. It will print all received messages to stdout.

## Usage

```
  ./example-nmea {OPTIONS}

    NMEA Example (3.4.0) - This is an example program that demonstrates how to
    use the NMEA receiver library. The program takes an interface and port
    associated with the receiver as arguments and print all received messages.

  OPTIONS:

      -?, --help                        Display this help menu
      -v, --version                     Display version information
      Interface:
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
        Stdin:
          --stdin                           Enable stdin
```
