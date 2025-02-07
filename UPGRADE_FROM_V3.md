# Upgrade Guide (v3.x -> v4.x)

The `example-lpp` client has been superseded by the more advanced `example-client`. This new client introduces several changes, including modified argument names and updated default settings. This guide outlines the key changes and enhancements to help you transition smoothly.

## Location Server Configuration

- **Host and Port Changes:**
  - The argument `--host` has been renamed to `--ls-host`.
  - The argument `--port` has been renamed to `--ls-port`.
  - Unlike in previous versions, `--ls-port` is now a required argument and does not default to 5431.

## Identity Management

- **Identity Specification:**
  - You must now specify one of the following options: `--imsi`, `--msisdn`, `--ipv4`, or `--wait-for-identity`.
  - The previous default for identity has been removed, making it necessary to explicitly provide one of these options.

## Assistance Data Configuration

- **Assistance Data Type:**
  - Use the `--ad-type` argument to specify the type of assistance data you wish to request. For example, to request OSR data, use `--ad-type=osr`.
  
- **Argument Changes:**
  - The argument `--nr` has been changed to `--nr-cell`.

## Input and Output System

The previous system supporting u-blox and NMEA receivers has been replaced with a more versatile input/output framework. To replicate the old `--nmea-serial` behavior, use the following commands:

- **Input Configuration:**
  ```bash
  --input serial:device=DEVICE,baudrate=115200,format=nmea
  ```

- **Output Configuration:**
  ```bash
  --output serial:device=DEVICE,baudrate=115200,format=rtcm
  ```

This new system is unified, allowing both Control and NMEA exports to operate through the same configuration framework.

## Data Processors

In the previous version, data transformation engines were selected based on whether you used `osr` or `ssr`. The new version separates these processing engines, allowing them to run simultaneously. The available processors are:

- **LPP to RTCM:**
  - Use the `--lpp2rtcm` processor if you previously used `osr`.

- **LPP to SPARTN:**
  - Use the `--lpp2spartn` processor if you previously used `ssr`.

- **Tokoro:**
  - This new processor generates OSR data from SSR inputs. Use the `--tokoro` argument to enable it.

