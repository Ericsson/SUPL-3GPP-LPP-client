# Interfaces

## Public Library Interfaces

### dependency::lpp — LPP Client API

Entry point: `dependency/lpp/include/lpp/`

```cpp
// Create and configure a client
lpp::Client client(scheduler, supl_host, supl_port, cell_info);

// Request assistance data (single session)
client.request_assistance_data(gnss_flags, callback);

// Request periodic assistance data
client.request_periodic_assistance_data(interval, gnss_flags, callback);

// Provide location information back to server
client.provide_location_information(location_info);
```

Key types:
- `lpp::Client` — top-level client
- `lpp::LocationInformation` — position + accuracy + velocity
- `lpp::HaGnssMetrics` — GNSS quality metrics
- `lpp::Optional<T>` — optional field wrapper

### dependency::supl — SUPL Protocol API

Entry point: `dependency/supl/include/supl/`

```cpp
supl::Session session(scheduler, host, port);
session.connect();
session.send_posinit(cell_info, capabilities);
// callbacks for received messages
```

### dependency::scheduler — Event Loop API

Entry point: `dependency/scheduler/include/scheduler/`

```cpp
scheduler::Scheduler scheduler;

// Schedule a task
scheduler.schedule(task);

// Defer a callback to next iteration
scheduler.defer([](){ /* ... */ });

// Run until cancelled
scheduler.run();
scheduler.cancel();
```

### dependency::io — I/O API

Entry point: `dependency/io/include/io/`

```cpp
// TCP client
auto tcp = io::TcpClient::create(scheduler, host, port);
tcp->on_data([](uint8_t* buf, size_t len){ /* ... */ });
tcp->connect();

// Serial port
auto serial = io::SerialPort::create(scheduler, "/dev/ttyUSB0", 115200);

// File
auto file = io::FileStream::create(scheduler, "output.bin", io::FileMode::Write);
```

### dependency::streamline — Pipeline API

Entry point: `dependency/streamline/include/streamline/`

```cpp
streamline::System system(scheduler);

// Register a processor
system.add_inspector<MyProcessor>(/* ctor args */);

// Push data into the pipeline
system.push(my_data, tag);

// Implement a processor
class MyProcessor : public streamline::Inspector<MyDataType> {
    void inspect(streamline::System& sys, MyDataType const& data, uint64_t tag) override;
    char const* name() const noexcept override { return "MyProcessor"; }
};
```

### dependency::coordinates — Coordinate API

Entry point: `dependency/coordinates/include/coordinates/`

```cpp
using WGS84 = coordinates::WGS84Frame;

coordinates::Ecef<WGS84> ecef{x, y, z};
coordinates::Llh<WGS84> llh = coordinates::ecef_to_llh(ecef);
coordinates::Enu<WGS84> enu = coordinates::ecef_to_enu(ecef, ref_ecef);
```

### dependency::gnss — GNSS Type API

```cpp
gnss::SatelliteId sat{gnss::System::GPS, 5};  // GPS PRN 5
gnss::SignalId sig{sat, gnss::Signal::L1CA};
```

### dependency::time — Time API

```cpp
ts::GpsTime gps{week, tow};
ts::UtcTime utc = ts::gps_to_utc(gps);
ts::TaiTime tai = ts::gps_to_tai(gps);
```

### dependency::ephemeris — Ephemeris API

```cpp
ephemeris::GpsEphemeris eph = /* decoded from LPP */;
auto result = eph.compute_position(gps_time);
// result.position: Ecef<WGS84>
// result.clock_correction: double
```

## Generator Interfaces

### generator::rtcm

```cpp
// Build RTCM MSM7 observation messages
rtcm::Generator gen;
gen.set_reference_station(station_id, ecef);
auto messages = gen.generate_msm7(observations, epoch_time);
```

### generator::spartn

```cpp
spartn::Generator gen;
auto ocb = gen.generate_ocb(orbit_clock_bias_data);
auto hpac = gen.generate_hpac(ionosphere_troposphere_data);
```

### generator::tokoro

Tokoro generates OSR (Observation Space Representation) corrections from SSR (State Space Representation) LPP data.

```cpp
tokoro::Generator gen(config);
gen.set_reference_location(llh);
auto rtcm_messages = gen.process(lpp_assistance_data, receiver_observations);
```

## Format Parser Interfaces

### format::nmea

```cpp
format::nmea::Parser parser;
parser.on_gga([](format::nmea::GGA const& msg){ /* ... */ });
parser.feed(buffer, length);
```

### format::ubx

```cpp
format::ubx::Parser parser;
parser.on_nav_pvt([](format::ubx::NavPvt const& msg){ /* ... */ });
parser.feed(buffer, length);
```

### format::rtcm

```cpp
format::rtcm::Parser parser;
parser.on_msm7([](format::rtcm::Msm7 const& msg){ /* ... */ });
parser.feed(buffer, length);
```

### format::at

```cpp
format::at::Parser parser;
parser.on_response([](format::at::Response const& r){ /* ... */ });
parser.feed(buffer, length);
```

## Logging Interface

```cpp
LOGLET_MODULE("my_module");  // declare at file scope

// In functions:
DEBUGF("value: %d", x);
INFOF("connected to %s", host);
WARNF("retry %d", attempt);
ERRORF("failed: %s", error.code_name());
VERBOSEF("detailed: %p", ptr);

// Scope tracing:
VSCOPE_FUNCTION();           // logs entry/exit of current function
VSCOPE_FUNCTIONF("id=%d", id);
```

## Error Handling Interface

```cpp
// Return errors
Error my_func() {
    if (bad) return make_error(ErrorDomain::IO, MY_ERROR_CODE);
    return ok();
}

// Return values with errors
Result<int> my_func2() {
    if (bad) return make_error(ErrorDomain::IO, MY_ERROR_CODE);
    return 42;
}

// Check results
auto result = my_func2();
if (!result.ok()) {
    ERRORF("failed: %s", result.error().code_name());
    return result.error();
}
int value = result.value();
```
