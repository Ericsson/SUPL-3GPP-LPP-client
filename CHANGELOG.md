# Changelog

## [3.4.*]
- You can now set the provide location information update rate with `--update-rate`. 
- Added more options for SPARTN generation:
    - `--no-code-bias-translate` to never translate between signal biases.
    - `--no-code-bias-correction-shift` to never apply correction shift to code biases.
    - `--no-phase-bias-translate` to never translate between signal biases.
    - `--no-phase-bias-correction-shift` to never apply correction shift to phase biases.
    - `--hydrostatic-in-zenith` to include hydrostatic delay residuals in the zenith residuals.
    - `--no-gps` to skip generating GPS SPARTN messages.
    - `--no-glonass` to skip generating GLONASS SPARTN messages.
    - `--no-galileo` to skip generating Galileo SPARTN messages.
    - `--beidou` to include generating BeiDou SPARTN messages.
    - `--flip-grid-bitmask` to flip incoming LPP grid bitmask.
- Deprecated the following options:
    - `--iode-shift` removed as it was not used.
    - `--ublox-clock-correction` is now the default behavior, use `--no-ublox-clock-correction` to disable it.
    - `--force-continuity` is now the default behavior, use `--no-force-continuity` to disable it.
    - `--average-zenith-delay` is now the default behavior, use `--no-average-zenith-delay` to disable it.
- Removed `from_ellipse` and replaced it with:
    - `to_ellipse_39` to create a horizontal accuracy ellipse with 39% confidence, will not rescale the axes.
    - `to_ellipse_68` to create a horizontal accuracy ellipse with 68% confidence, will rescale the axes.
- RTCM 1006 physical reference station indicator was incorrectly set to 0 for "virtual" stations and 1 for "physical" stations, instead of the other way around. 
- RTCM 1005 physical reference station indicator was incorrectly set to 0 for "virtual" stations and 1 for "physical" stations, instead of the other way around.
- Added options for `example-lpp`:
    - `--no-gps` to skip generating GPS RTCM messages.
    - `--no-glonass` to skip generating GLONASS RTCM messages.
    - `--no-galileo` to skip generating Galileo RTCM messages.
    - `--no-beidou` to skip generating BeiDou RTCM messages.
- Added receiver option `--readonly` to prevent opening receiver serial port in write mode.
- Added support to export NMEA sentences to file with `--nmea-export-file` command-line argument.
- Added support to export UBX messages to file with `--ublox-export-file` command-line argument.

## [3.4.11] 2024-09-26
- Added support for UBX-RXM-RAWX messages.
- Added support for $PQTMEPE messages.

## [3.4.10] 2024-08-07
- Added new control command `/IDENTITY` to provide the client with the IMSI, MSISDN, or IP address. See `CONTROL.md` for more information. 
- Added new option `--wait-for-identity` to have the client wait for an identity before sending any assistance data requests.
- Added new example `example-modem-ctrl` to demonstrate how to send control commands of cell IDs and IMSI to the client using the control interface.
- Fixed a bug where `/CID` (or any update assistance data request) would leak memory if the client couldn't encode the message.
- Updated modem library and `example-lpp` is no longer using it. Use control commands to update cell IDs and IMSI instead.
- Added options to use the SLP address instead of providing `--host`.
    - `--slp-host-imsi` to use the IMSI to generate the SLP address.
    - `--slp-host-cell` to use the cell information to generate the SLP address.
- Fixed a bug where the default `ura-override` value was initialized to 0 causing the SPARTN generator to always include it as "unknown".
- Updated example location implementation in the LPP example to match the new version of the LocationInformation struct (#28 by Phillezi)
- Added option `--lrf-message-id` to specify the RTCM message type to be used with `--format lrf-uper`.
- Added TCP and UDP support when using the NMEA receiver.
    - `--nmea-tcp` to specify the TCP ip address to connect to.
    - `--nmea-tcp-port` to specify the TCP port to connect to.
- The encoding of ha-uncertainty has been changed to the closed value and will never be encoded as 0.0m.
- Fixed bug where std::stoull would throw an exception and crash the client

## [3.4.9] 2024-05-03
- Added a few options to controll how SPARTN messages are generated:
    - `--sf055-default` to set the default value for the ionospheric quality if not available.
    - `--sf042-override` to force the tropospheric quality to a specific value (previous `--ura-override` was also used for this).
    - `--sf042-default` to set the default value for the tropospheric quality if not available.
    - `--filter-by-ocb` to filter out ionopsheric residuals for satellites not in the OCB.
    - `--ignore-l2l` to ignore L2L biases. 
- Fixed a bug where parsing NMEA GGA would fail if age of corrections was not provided.

## [3.4.8] 2024-04-26
- Added support for age of correction when using NMEA.

## [3.4.7] 2024-04-25
- Added support for UBX-RXM-RTCM messages.
- Added support for UBX-RXM-SPARTN messages.
- Added new format option `lrf-uper` to output RTCM framed UPER encoded 3GPP LPP messages.
- Updated 3GPP LPP version from Release 16.4.0 to Release 18.1.0.
- HA-GNSS-Metrics data is now included in the `ProvideLocationInformation` message.
- Fixed memory leak from ProvideLocationInformation.

## [3.4.6] 2024-04-04
- You can optionally include/exclude which generators to build by using the CMake options `-DINCLUDE_GENERATOR_*`. By default, RTCM and SPARTN generators are included and the old SPARTN generator is excluded.
- Added support for control commands in the `example-lpp` when using `osr`, this was previously only available in when using `ssr`.

## [3.4.5] 2024-04-02
- SPARTN generator will not use provided URA epoch-time. This caused issues where only the URA timestamp would be used and because it isn't update vary frequently the corrections data would not be used.
- SPARTN is now "officially" supported. It has been tested with multiple data feed providers and is working as expected.
- Added option to override the ionospheric quality indiciator (SF055).
- Added support for external control commands. This allows the client to be controlled by an external application. See `CONTROL.md` for more information.
- Building with OpenSSL support is now off by default. To enable it, use `-DUSE_OPENSSL=ON` when building with CMake.
- Experimental changes can now be toggled with `-DEXPERIMENTAL=ON` when building with CMake.

## [3.4.4] 2024-03-06   
- Fixed message nullptr exception.
- IMSI and MSISDN used `unsigned long` which doesn't have enough bits to store all possible values. Changed to `unsigned long long`.
- Added support to use 5G NR cells in addition to LTE cells.
- Reworked `LocationInformation` structure to be more aligned to the 3GPP LPP specification. It now supports more location coordinate and velocity types. All example function to fill in the `LocationInformation` structure have been updated to reflect this change.
- Changed `--location-info` to `--fake-location-info` (or `-fli`).
- Changed `--latitude` to `--fake-latitude` (or `-flat`).
- Changed `--longitude` to `--fake-longitude` (or `-flon`).
- Changed `--altitude` to `--fake-altitude` (or `-falt`).
- Added `--location-report-unlocked` to always use the server provided location information update interval. Otherwise, the update interval will be set locked to 1 second.
- Added support to parse request periodicity from RequestLocationInformation message.
- When using the u-Blox receiver in `example-lpp`, it will not load the receiver configuration. This behavior is unchanged for `example-ublox` but can still be disabled with `--disable-config`.

## [3.4.3] 2024-02-08
- Added option to export NMEA sentences to a unix socket or a TCP connection. See `--nmea-export-*` command-line arguments. This requires the use of the NMEA receiver.
- Fixed a bug where in cases of TCP connection failure, the addrinfo struct was not freed.

## [3.4.2] 2024-02-02
- Fixed a bug where GAD messages had TF009 set to 0. It will now be set to time specified in the STEC/Gridded IE.
- Fixed a bug where parsing bitfields in UBX-NAV-PVT would not be incorrect.

## [3.4.1] 2024-01-30
- Fixed a crash due to missing null pointer check in NMEA `ThreadedReceiver`.

## [3.4.0] 2024-01-26
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
