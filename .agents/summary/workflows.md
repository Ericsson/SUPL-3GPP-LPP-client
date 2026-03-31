# Workflows

## 1. LPP Assistance Data Request (Single Session)

```mermaid
sequenceDiagram
    participant App as Application
    participant Client as lpp::Client
    participant Session as lpp::Session
    participant SUPL as supl::Session
    participant Server as SUPL Server

    App->>Client: request_assistance_data(gnss_flags, callback)
    Client->>Session: create SingleSession
    Session->>SUPL: connect(host, port)
    SUPL->>Server: TCP connect + TLS handshake
    Server-->>SUPL: connected
    SUPL->>Server: SUPL POSINIT (cell info + capabilities)
    Server-->>SUPL: SUPL POS (LPP RequestAssistanceData)
    Session->>Server: LPP ProvideCapabilities
    Server-->>Session: LPP ProvideAssistanceData
    Session->>App: callback(assistance_data)
    Session->>Server: LPP Abort / session end
    SUPL->>Server: SUPL END
```

## 2. Periodic Assistance Data Delivery

```mermaid
sequenceDiagram
    participant App as Application
    participant Client as lpp::Client
    participant PeriodicSession as lpp::PeriodicSession
    participant Server as SUPL Server

    App->>Client: request_periodic_assistance_data(interval, flags, callback)
    Client->>PeriodicSession: create
    PeriodicSession->>Server: SUPL POSINIT + LPP session setup
    loop Every interval
        Server-->>PeriodicSession: LPP ProvideAssistanceData
        PeriodicSession->>App: callback(assistance_data)
    end
    App->>Client: stop_periodic_session()
    Client->>Server: LPP Abort + SUPL END
```

## 3. LPP → RTCM Conversion Pipeline (example-client)

```mermaid
flowchart LR
    LPP[LPP Assistance Data] --> Streamline[streamline::System]
    Streamline --> Lpp2Rtcm[Lpp2Rtcm Inspector]
    Streamline --> Lpp2Spartn[Lpp2Spartn Inspector]
    Streamline --> Tokoro[Tokoro Inspector]
    Streamline --> NmeaOut[NMEA Output Inspector]
    Lpp2Rtcm --> IO[I/O Output\nSerial / TCP / UDP / File]
    Lpp2Spartn --> IO
    Tokoro --> IO
    NmeaOut --> IO
```

## 4. Tokoro SSR→OSR Correction Generation

```mermaid
flowchart TD
    LPP[LPP ProvideAssistanceData] --> Gather[Gather SSR Data\norbit/clock/bias/STEC/tropo]
    Gather --> EphStore[Ephemeris Store\nGPS/GAL/BDS/GLO/QZS]
    Gather --> CorrStore[Correction Store\nper satellite per epoch]
    RecvObs[Receiver Observations\nUBX NAV-PVT / NMEA] --> RefLoc[Reference Location]
    RefLoc --> Tokoro[tokoro::Generator]
    EphStore --> Tokoro
    CorrStore --> Tokoro
    Tokoro --> OrbitCorr[Orbit Correction]
    Tokoro --> ClockCorr[Clock Correction]
    Tokoro --> BiasCorr[Code/Phase Bias]
    Tokoro --> TropoCorr[Troposphere Correction]
    Tokoro --> IonoCorr[Ionosphere STEC]
    OrbitCorr --> RTCM[RTCM MSM7 + SSR Messages]
    ClockCorr --> RTCM
    BiasCorr --> RTCM
    TropoCorr --> RTCM
    IonoCorr --> RTCM
```

## 5. example-client Startup Sequence

```mermaid
flowchart TD
    Start[main()] --> ParseCLI[Parse CLI args\nargs.hxx]
    ParseCLI --> CreateScheduler[Create Scheduler]
    CreateScheduler --> SetupIO[Setup I/O endpoints\nserial/TCP/UDP/file]
    SetupIO --> CreateStreamline[Create streamline::System]
    CreateStreamline --> AddProcessors[Add Processors\nbased on config flags]
    AddProcessors --> CreateLPP[Create lpp::Client]
    CreateLPP --> ConnectSUPL[Connect to SUPL server]
    ConnectSUPL --> RunLoop[scheduler.run()\nevent loop]
    RunLoop --> |LPP data arrives| Pipeline[Push to streamline pipeline]
    Pipeline --> |processed| Output[Write to configured outputs]
```

## 6. Format Parser Data Flow

```mermaid
flowchart LR
    Serial[Serial Port\nor TCP stream] --> IOStream[io::Stream\nbuffered read]
    IOStream --> Parser[format::Parser\nnmea/ubx/rtcm/at]
    Parser --> |parsed message| Callback[User callback]
    Callback --> Streamline[streamline::System\npush typed message]
```

## 7. Build Workflow

```mermaid
flowchart LR
    Src[Source Code] --> CMake[cmake -GNinja\n-DCMAKE_BUILD_TYPE=Debug\n-DBUILD_EXAMPLES=ON]
    CMake --> Configure[Configure\nresolve flags\nfetch Eigen3 via CPM]
    Configure --> Ninja[ninja]
    Ninja --> Libs[Static Libraries\ndependency_*]
    Ninja --> Bins[Binaries\nexample-client\nexample-ntrip\n...]
```

## 8. Test Execution

```mermaid
flowchart LR
    Build[ninja\nwith BUILD_TESTING=ON] --> CTest[ctest --output-on-failure]
    CTest --> UnitTests[Unit Tests\ndoctest framework]
    UnitTests --> Coords[coordinates tests]
    UnitTests --> Time[time tests]
    UnitTests --> Eph[ephemeris tests]
    UnitTests --> IO[io tests]
    UnitTests --> Scheduler[scheduler tests]
    UnitTests --> Msgpack[msgpack tests]
    UnitTests --> Error[error tests]
    UnitTests --> LPP[lpp tests]
    UnitTests --> GNSS[gnss tests]
```

## 9. Fuzzing Workflow

```mermaid
flowchart LR
    Build[ninja\nwith ENABLE_FUZZING=ON\nClang + libFuzzer] --> FuzzTargets[fuzz_nmea\nfuzz_ubx\nfuzz_rtcm\nfuzz_at]
    FuzzTargets --> Corpus[tests/corpus/\nnmea / ubx / rtcm / at]
    Corpus --> |seed inputs| FuzzRun[./fuzz_nmea corpus/nmea\n-max_total_time=60]
    FuzzRun --> |new inputs| Corpus
```
