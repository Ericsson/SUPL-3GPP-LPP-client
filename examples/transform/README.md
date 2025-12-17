# Transform Example

Coordinate transformation tool for converting between different coordinate systems and reference frames.

## Usage

```bash
./example-transform [options]
```

The tool reads coordinate transformation commands from stdin, one per line.

## Input Format

### Basic Coordinate Transformation
```
<type> <v1> <v2> <v3> [options]
```

**Coordinate Types:**
- `ecef` - Earth-Centered Earth-Fixed (meters)
- `llh` - Latitude, Longitude, Height (degrees, degrees, meters)
- `enu` - East, North, Up (meters, requires origin)
- `ned` - North, East, Down (meters, requires origin)
- `aer` - Azimuth, Elevation, Range (degrees, degrees, meters, requires origin)

### Options

**Output Type:**
```
-> <type>
```
Convert to specified coordinate type (default: same as input)

**Reference Frames:**
```
from <frame> to <frame>
```
Transform between reference frames (e.g., `from wgs84 to itrf2020`)

**Epochs:**
```
@ <year>        # Input epoch (decimal year)
@out <year>     # Output epoch (decimal year)
```

**Velocity:**
```
vel <vx> <vy> <vz>
```
Velocity in meters/year (ECEF components)

**Set Origin:**
```
origin llh <lat> <lon> <height>
```
Set origin for local coordinate systems (ENU/NED/AER)

## Examples

### Basic Conversions
```bash
# ECEF to LLH
echo "ecef 4027893.87 23717.25 4916287.22 -> llh" | ./example-transform

# LLH to ECEF
echo "llh 51.5 -0.1 100 -> ecef" | ./example-transform
```

### Reference Frame Transformations
```bash
# WGS84 to ITRF2020 at epoch 2020.0
echo "llh 51.5 -0.1 100 from wgs84 to itrf2020 @ 2020.0" | ./example-transform
```

### Local Coordinates
```bash
# Set origin and convert to ENU
echo "origin llh 51.5 -0.1 0" | ./example-transform
echo "llh 51.51 -0.09 50 -> enu" | ./example-transform
```

### With Velocity
```bash
# Transform with velocity (plate tectonics)
echo "ecef 4027893.87 23717.25 4916287.22 vel 0.01 0.02 0.03 from itrf2014 to itrf2020 @ 2015.0 @out 2020.0" | ./example-transform
```

## Command-Line Options

- `-r, --origin <lat,lon,h>` - Set origin for local coordinates
- `-p, --path` - Show transformation path with intermediate steps
- `-l, --list` - List available reference frames
- `-m, --matrix` - Show transformation matrix
- `--trace/--verbose/--debug/--info/--warning/--error` - Set log level
- `--log-no-color` - Disable colored output
- `--log-flush` - Flush log after each line

## Available Reference Frames

Use `--list` to see all available frames. Common frames include:
- `wgs84` (alias for wgs84(g1762))
- `itrf2020`, `itrf2014`, `itrf2008`, etc.
- `nad83(2011)`, `etrs89`, `gda2020`

## Interactive Mode

Run without input to enter interactive mode:
```bash
./example-transform
origin llh 51.5 -0.1 0
llh 51.51 -0.09 50 -> enu
llh 51.49 -0.11 30 -> enu
^D
```
