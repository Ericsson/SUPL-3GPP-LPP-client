# NTRIP Caster Example

A minimal NTRIP caster with a single mountpoint. It reads correction data from
any of the standard `--input` sources (file, serial, TCP, UDP, stdin, ...)
and broadcasts the raw bytes to every connected NTRIP client.

Supports both NTRIP 1.0 (bare `GET /mountpoint` request, `ICY 200 OK`
response) and NTRIP 2.0 (HTTP/1.1 request with headers). Serves a
sourcetable when a client requests `/`, and optionally requires a
username/password (HTTP Basic authentication) to access the mountpoint.

## Usage

```
./example-ntrip-caster --input <source> {OPTIONS}
```

Example — broadcast a file's contents to clients connecting to
`/STREAM` on port 2101:

```
./example-ntrip-caster --input file:path=data.rtcm3 --mountpoint STREAM --port 2101
```

Example — require authentication:

```
./example-ntrip-caster --input tcp-client:host=127.0.0.1+port=6000 \
    --mountpoint STREAM --username alice --password secret
```

Connect with curl to fetch the sourcetable:

```
curl http://127.0.0.1:2101/
```

Connect with curl to stream the mountpoint:

```
curl --ntrip-version ntrip1 http://alice:secret@127.0.0.1:2101/STREAM
```

## Options

### Caster
- `--address` — Listen address (default: `0.0.0.0`)
- `--port` — Listen port (default: `2101`)
- `--username` / `--password` — Require HTTP Basic authentication to access
  the mountpoint. Both must be set together; if unset, the mountpoint is
  open to anyone.

### Mountpoint (sourcetable `STR;` fields)
- `--mountpoint` — Mountpoint name (default: `STREAM`)
- `--identifier` — Identifier field
- `--format` — Data format field (default: `RTCM 3.3`)
- `--format-details` — Format details field
- `--carrier` — Carrier field (default: `2`)
- `--nav-system` — Navigation system field
- `--network` — Network field
- `--country` — Country field (default: `SWE`)
- `--latitude` / `--longitude` — Approximate reference position
- `--nmea` — Mark the mountpoint as requiring an NMEA GGA sentence from the
  client (informational only — the caster does not enforce this)
- `--solution` — Mark the mountpoint as a network RTK solution
- `--generator` — Generator field
- `--bitrate` — Bitrate field (default: `0`)
- `--misc` — Misc field

### Input
Any `--input` source supported by the client-io library (`file:`, `serial:`,
`tcp-client:`, `tcp-server:`, `udp-server:`, `stdin:`, ...). At least one
`--input` is required. Run `--help` for the full list of input types and
their options.

## Notes
- This is a single-mountpoint caster intended for testing and development.
  It does not support chunked transfer, GGA position updates from clients,
  or multiple simultaneous mountpoints.
- Authentication, when enabled, is checked via `Authorization: Basic` only
  (no digest auth). Credentials are sent in cleartext unless an external TLS
  terminator is used in front of the caster.
