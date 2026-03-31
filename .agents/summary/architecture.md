# Architecture

## System Overview

SUPL-3GPP-LPP-client is a C++ toolkit that implements a 3GPP LPP client. It connects to a SUPL server over TCP/TLS, exchanges LPP messages to obtain GNSS assistance data, and can convert that data to RTCM or SPARTN correction formats for downstream GNSS receivers.

```mermaid
graph TB
    subgraph Application["Example Application (example-client)"]
        Main["main.cpp\nCLI + orchestration"]
        Processors["Processors\n(Tokoro, Idokeido, NMEA, etc.)"]
    end

    subgraph Protocol["Protocol Stack"]
        LPP["dependency::lpp\nLPP client + sessions"]
        SUPL["dependency::supl\nSUPL encode/decode"]
        ASN1["external::asn1\nASN.1 codec"]
    end

    subgraph Transport["Transport & I/O"]
        Scheduler["dependency::scheduler\nepoll event loop"]
        IO["dependency::io\nSerial / TCP / UDP / File"]
    end

    subgraph Conversion["Message Conversion"]
        GenRTCM["generator::rtcm\nLPP→RTCM"]
        GenSPARTN["generator::spartn\nLPP→SPARTN"]
        GenTokoro["generator::tokoro\nSSR corrections"]
        GenIdokeido["generator::idokeido\nPPP corrections"]
    end

    subgraph Support["Support Libraries"]
        Coords["dependency::coordinates\nECEF/LLH/ENU/NED/AER"]
        Eph["dependency::ephemeris\nGPS/GAL/BDS/GLO/QZS"]
        Time["dependency::time\nGPS/UTC/TAI/BDT/GST/GLO"]
        GNSS["dependency::gnss\nSatelliteId / SignalId"]
        Format["dependency::format\nNMEA/UBX/RTCM/AT parsers"]
        Streamline["dependency::streamline\nTyped pub/sub pipeline"]
    end

    Main --> LPP
    Main --> Processors
    Processors --> Streamline
    LPP --> SUPL
    SUPL --> ASN1
    LPP --> Scheduler
    IO --> Scheduler
    Processors --> GenRTCM
    Processors --> GenSPARTN
    Processors --> GenTokoro
    Processors --> GenIdokeido
    GenTokoro --> Coords
    GenTokoro --> Eph
    GenTokoro --> Time
    GenRTCM --> GNSS
    Format --> GNSS
```

## Layered Architecture

```mermaid
graph BT
    L1["Layer 1: Core\ncore · loglet · maths · msgpack"]
    L2["Layer 2: Domain Types\ngnss · time · coordinates · ephemeris"]
    L3["Layer 3: I/O & Scheduling\nio · scheduler · streamline · modem"]
    L4["Layer 4: Protocols\nsupl · lpp · format"]
    L5["Layer 5: Generators\ngenerator/rtcm · spartn · tokoro · idokeido"]
    L6["Layer 6: Applications\nexamples/client · ntrip · transform · lpp2spartn"]

    L2 --> L1
    L3 --> L1
    L4 --> L2
    L4 --> L3
    L5 --> L4
    L5 --> L2
    L6 --> L5
    L6 --> L4
```

## Key Design Patterns

### Streamline Pipeline (Pub/Sub)

The `streamline::System` is a type-indexed publish/subscribe bus backed by the scheduler's event loop. Components register as `Inspector<T>` (read-only) or `Consumer<T>` (exclusive). Producers call `system.push(data, tag)`.

```mermaid
classDiagram
    class System {
        +add_inspector~Inspector~()
        +add_consumer~Consumer~()
        +push~DataType~(data, tag)
    }
    class Inspector~T~ {
        <<abstract>>
        +inspect(System, data, tag)
        +accept(System, tag) bool
        +name() const char*
    }
    class Consumer~T~ {
        <<abstract>>
        +consume(System, data, tag)
        +name() const char*
    }
    System --> Inspector
    System --> Consumer
```

### Error Handling

All fallible operations return `Error` or `Result<T>`. No exceptions are thrown.

```mermaid
classDiagram
    class Error {
        +ErrorDomain domain
        +ErrorCodeBase code
        +int system_errno
        +ok() bool
        +operator bool()
        +none()$ Error
    }
    class ErrorDomain {
        <<enum>>
        Generic
        Scheduler
        IO
        LPP
        SUPL
        Format
        Modem
    }
    Error --> ErrorDomain
```

### Coordinate System (Type-Safe Frames)

Coordinate types are parameterized by a `Frame` type that carries ellipsoid data, preventing accidental mixing of reference frames at compile time.

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
    }
    Ecef --> Llh : ecef_to_llh()
    Llh --> Ecef : llh_to_ecef()
    Ecef --> Enu : ecef_to_enu()
```

### LPP Session State Machine

```mermaid
stateDiagram-v2
    [*] --> DISCONNECTED
    DISCONNECTED --> CONNECTED : TCP connect
    CONNECTED --> ESTABLISHED : SUPL POSINIT handshake
    ESTABLISHED --> ESTABLISHED : LPP message exchange
    ESTABLISHED --> DISCONNECTED : session end / error
    CONNECTED --> DISCONNECTED : error
```

## CMake Module Pattern

Every dependency follows this pattern:

```cmake
add_library(dependency_<name> STATIC <sources>)
add_library(dependency::<name> ALIAS dependency_<name>)
target_include_directories(dependency_<name> PUBLIC include/)
target_link_libraries(dependency_<name> PUBLIC <deps>)
setup_target(dependency_<name>)
```

`setup_target()` (defined in `cmake/target.cmake`) applies consistent compiler flags, warnings, and optional static analysis.
