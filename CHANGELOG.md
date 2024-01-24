# Changelog

## [3.4.0]
- Support for receivers that communicate using NMEA protocol has been added. The following sentences are now supported:
    - GGA
    - GST
    - VTG
- A new example has been added to demonstrate how to use the NMEA receiver.
- A command-line argument `--print-receiver-messages` (or `--prm`) has been added to print all received messages to the standard output. This option is valid for any receiver (NMEA or u-Blox).
- A bug related to ASN.1 BIT STRING encoding has been fixed. The MSB and LSB were swapped. This should not affect OSR or SSR assistance data request.
- A bug where the SUPL identity was not properly encoded when using IMSI and MSISDN has been fixed. A value for ABCD was encoded as BADC. The fix is only active when the argument `--supl-identity-fix` is specified.
- A way to send a "fake" location has been added with the command-line arguments `--location-info`, `--latitude`, `--longitude`, and `--altitude`. There is also an option to always send location information, even if the location server does not request it. This is done with the command-line argument `--force-location-info`.
- An option to disable printing the "RTCM: XXXX bytes | " when using OSR and RTCM generation has been added.
- SPARTN (new) and SPARTN (old) generators have been added. These generators can be used to convert SSR assistance data to SPARTN format.
