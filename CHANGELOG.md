# Changelog

## [3.4.0]
- Support for receivers that communicate using NMEA protocol has been added. The following sentences are now supported:
    - GGA
    - GST
    - VTG
- Added a new example has been added to demonstrate how to use the NMEA receiver.
- Added a command-line argument `--print-receiver-messages` (or `--prm`) to print all received messages to the standard output. This option is valid for any receiver (NMEA or u-Blox).
- Fixed a bug related to ASN.1 BIT STRING encoding. The MSB and LSB were swapped. This should not affect OSR or SSR assistance data request.
- Fixed a bug where the SUPL identity was not properly encoded when using IMSI and MSISDN. A value for ABCD was encoded as BADC. The fix is only active when the argument `--supl-identity-fix` is passed.
- Added way to send a "fake" location has been added with the command-line arguments `--location-info`, `--latitude`, `--longitude`, and `--altitude`. There is also an option to always send location information, even if the location server does not request it. This is done with the command-line argument `--force-location-info`.
- Added a option (`--rtcm-print`) to disable printing the "RTCM: XXXX bytes | " lines when using OSR and RTCM generation has been added.
- Added two SPARTN generators:
    - `spartn` - The original SPARTN generator.
    - `spartn2` - A new SPARTN generator that is based on the original SPARTN generator. It's recommended to use this generator instead of the original SPARTN generator.
- Added support to output ASN.1 UPER encoded 3GPP LPP ProvideAssistanceData messages from the `example_lpp`.
- Moved `build-with-gcc4.8.sh` to `docker/verify-with-gcc4.8.sh` and removed the use of `sudo` in the script.
- Changed to use `TAI_Time` instead of `time_t` to represent estimation time in the `LocationInformation` structure.
- `provide_location_information_callback_ublox` now sets the `location_info->time` field to the time received from the receiver instead of the current time.
- Fixed a bug where `--ublox-serial-data-bits` was used instead of `--serial-data-bits` for the output serial configuration.
- Fixed command-line argument `--version` (or `-v`) to _actually_ print the version.
- Added a new example `example_lpp2spartn` to demonstrate how to convert LPP to SPARTN. It takes ASN.1 UPER messages from STDIN and converts them to SPARTN messages.
- Added a new example `example_ntrip` which can be used to connect to an NTRIP caster and receiver RTCM messages.
- Added a new interface `stdin` that can be used to read data from STDIN.
- `ProvideLocationInformation` now uses MeasurementReferenceTime to provide a precise time estimate with a 250ns resolution.
- Fixed a bug where UBX messages would be missed due to the way we were reading from the interface.
- Fixed a bug where the bitfields in UBX-NAV-PVT were not properly right-shifted down to the LSB. `diff_soln` would report `0b00` or `0b10` instead of `0b0` or `0b1`.
