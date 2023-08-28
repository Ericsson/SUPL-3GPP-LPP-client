# Example - u-blox Receiver

This is an example program that demonstrates how to use the u-Blox receiver library. The program takes an interface and port associated with the receiver as arguments. It will the connect, configure the u-blox to output UBX-NAV-PVT, and print all received messages to stdout.

## Usage



```
./example-ublox {OPTIONS}

Example - u-blox Receiver

OPTIONS:

    -?, --help                        Display this help menu
    -v, --version                     Display version information
    Configuration:
      --disable-config                  Disable configuration
    Receiver:
      --port=[port]                     The port used on the u-blox receiver,
                                        used by configuration.
                                        One of: uart1, uart2, i2c, usb
                                        Default: (by interface)
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
```
