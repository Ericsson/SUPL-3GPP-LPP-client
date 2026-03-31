# Data Models

## Error Types

```mermaid
classDiagram
    class Error {
        +ErrorDomain domain
        +ErrorCodeBase code
        +int system_errno
        +ok() bool
        +operator bool()
        +domain_name() const char*
        +code_name() const char*
        +none()$ Error
    }
    class ErrorDomain {
        <<enum>>
        Generic = 0
        Scheduler = 1
        IO = 2
        LPP = 3
        SUPL = 4
        Format = 5
        Modem = 6
    }
    class ResultT {
        +ok() bool
        +value() T
        +error() Error
        +take_value() T
        +take_error() Error
    }
    Error --> ErrorDomain
    ResultT --> Error
```

## GNSS Types

```mermaid
classDiagram
    class SatelliteId {
        +System system
        +uint8_t prn
        +is_valid() bool
        +to_string() const char*
    }
    class System {
        <<enum>>
        GPS
        GLONASS
        Galileo
        BeiDou
        QZSS
        SBAS
        NavIC
    }
    class SignalId {
        +SatelliteId satellite
        +Signal signal
    }
    class Signal {
        <<enum>>
        L1CA
        L1P
        L2C
        L5
        E1
        E5a
        B1I
        ...
    }
    SatelliteId --> System
    SignalId --> SatelliteId
    SignalId --> Signal
```

## Coordinate Types

```mermaid
classDiagram
    class Ecef~Frame~ {
        +Vector3d value
        +x() double
        +y() double
        +z() double
    }
    class Llh~Frame~ {
        +Vector3d value
        +lat() double
        +lon() double
        +alt() double
    }
    class Enu~Frame~ {
        +Vector3d value
        +east() double
        +north() double
        +up() double
    }
    class Ned~Frame~ {
        +Vector3d value
    }
    class Aer {
        +double azimuth
        +double elevation
        +double range
    }
    class Ellipsoid {
        +double a
        +double f
        +double b
        +double e2
        +from_a_f()$ Ellipsoid
    }
```

## Time Types

```mermaid
classDiagram
    class GpsTime {
        +int32_t week
        +double tow
        +is_valid() bool
    }
    class UtcTime {
        +int64_t unix_ms
        +leap_seconds() int
    }
    class TaiTime {
        +int64_t tai_ms
    }
    class BdtTime {
        +int32_t week
        +double tow
    }
    class GstTime {
        +int32_t week
        +double tow
    }
    class GloTime {
        +int day_of_year
        +double tod
    }
    class Timestamp {
        +int64_t ns
    }
```

## LPP Location Information

```mermaid
classDiagram
    class LocationInformation {
        +LocationShape shape
        +Optional~HorizontalAccuracy~ h_accuracy
        +Optional~VerticalAccuracy~ v_accuracy
        +Optional~VelocityShape~ velocity
        +Optional~HaGnssMetrics~ gnss_metrics
        +GpsTime time
    }
    class HorizontalAccuracy {
        +double semi_major
        +double semi_minor
        +double orientation
        +double confidence
    }
    class VerticalAccuracy {
        +double value
        +double confidence
    }
    class HaGnssMetrics {
        +uint8_t num_satellites
        +double hdop
        +double vdop
    }
    class LocationShape {
        <<enum>>
        EllipsoidPoint
        EllipsoidPointWithUncertaintyCircle
        EllipsoidPointWithUncertaintyEllipse
        EllipsoidPointWithAltitude
        EllipsoidPointWithAltitudeAndUncertaintyEllipsoid
    }
    LocationInformation --> HorizontalAccuracy
    LocationInformation --> VerticalAccuracy
    LocationInformation --> HaGnssMetrics
    LocationInformation --> LocationShape
```

## Ephemeris Types

```mermaid
classDiagram
    class GpsEphemeris {
        +SatelliteId satellite
        +GpsTime toe
        +GpsTime toc
        +double sqrt_a
        +double e
        +double i0
        +double omega0
        +double omega
        +double m0
        +double delta_n
        +double idot
        +double omega_dot
        +double af0
        +double af1
        +double af2
        +double tgd
        +uint8_t ura
        +uint8_t iode
        +compute_position(GpsTime) EphemerisResult
    }
    class EphemerisResult {
        +Ecef position
        +Ecef velocity
        +double clock_correction
        +double relativity
    }
    GpsEphemeris --> EphemerisResult
```

## RTCM Data Types (format/rtcm)

The RTCM parser uses strongly-typed data fields defined in `datatypes.hpp`:

- `df_uint<N>` — unsigned N-bit field
- `df_int<N>` — signed N-bit field
- `df_intS<N>` — signed scaled integer
- `df_bit` — single bit flag

These wrap raw integer values with type safety to prevent accidental mixing of RTCM data fields.

## SPARTN Data Structures (generator/spartn)

```mermaid
classDiagram
    class OcbData {
        +GpsTime epoch_time
        +list~OcbSatellite~ satellites
    }
    class OcbSatellite {
        +SatelliteId id
        +OrbitCorrection orbit
        +ClockCorrection clock
        +list~CodeBias~ code_biases
        +list~PhaseBias~ phase_biases
    }
    class HpacData {
        +GpsTime epoch_time
        +list~HpacSatellite~ satellites
        +CorrectionPointSet grid
    }
    class CorrectionPointSet {
        +double lat_origin
        +double lon_origin
        +double lat_spacing
        +double lon_spacing
        +int rows
        +int cols
    }
    OcbData --> OcbSatellite
    HpacData --> HpacSatellite
    HpacData --> CorrectionPointSet
```

## MessagePack Serialization

Used for test data and Tokoro snapshots. The `msgpack` module provides:

- `MsgpackWriter` — sequential write of typed values (uint8, uint16, uint32, uint64, float, double, bytes)
- `MsgpackReader` — sequential read with type checking
- `MsgpackVector` — vector serialization helper

Test data files are stored in `tests/data/` (GPS, GAL, BDS, GLO, QZSS ephemeris in MessagePack format).
