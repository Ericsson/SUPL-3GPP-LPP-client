#!/usr/bin/env python3
import georinex as gr
import numpy as np
from pyrtklib import geph2pos, gpst2time, gpst2utc, utc2gpst, time2str, geph_t, gtime_t, Arr1Ddouble, Arr1Dchar, tracelevel, traceopen
import msgpack
import warnings
import sys
warnings.filterwarnings('ignore')

traceopen('rtklib_trace.txt')
tracelevel(5)

GPS_TO_UNIX_OFFSET = 315964800
GLONASS_EPOCH_GPS_DAYS = 5839  # days from GPS epoch to GLONASS epoch
FOUR_YEAR_PERIOD = 1461  # days in 4-year period
VALIDITY_PERIOD = 30 * 60
NUM_TESTS = 2
LEAP_SECONDS = 18  # current leap seconds (GPS - UTC)

def get_gps_time(dt):
    # georinex gives UTC time for GLONASS, convert to GPS time
    gps_epoch = np.datetime64('1980-01-06T00:00:00')
    utc_sec = int((dt - gps_epoch) / np.timedelta64(1, 's'))
    return utc_sec + LEAP_SECONDS  # UTC + leap seconds = GPS time

def gps_to_glo_timestamp(gps_sec):
    # Convert GPS to GLONASS matching C++ ts::Glo{ts::Gps{...}} conversion
    # Step 1: GPS to UTC (gps_2_utc in gps.cpp)
    # GPS timestamp is seconds since GPS epoch (1980-01-06)
    # UTC timestamp is seconds since Unix epoch (1970-01-01)
    # Offset: GPS epoch is 315964800 + 432000 + 172800 + 315360000 seconds after Unix epoch
    # = 10 years + 5 days + 2 leap days = 315964800 seconds
    GPS_TO_UNIX_OFFSET = 315964800
    utc_sec = gps_sec + GPS_TO_UNIX_OFFSET - LEAP_SECONDS
    
    # Step 2: UTC to GLONASS (utc_2_glo in glo.cpp)
    glo_sec = utc_sec
    glo_sec += 3 * 3600  # Add 3 hours (Moscow time)
    glo_sec -= 6 * 86400  # Subtract 6 leap days between 1970 and 1996
    glo_sec -= 26 * 365 * 86400  # Subtract 26 years (1996 - 1970)
    
    return [int(glo_sec), 0.0]

def build_glo_eph(eph_data, slot):
    eph = geph_t()
    
    eph.sat = slot + 100
    eph.frq = int(eph_data['FreqNum'].values)
    eph.svh = int(eph_data['health'].values)
    eph.age = int(eph_data['AgeOpInfo'].values) if 'AgeOpInfo' in eph_data else 0
    
    toe_gps = get_gps_time(np.datetime64(eph_data.time.values, 'ns'))
    week = toe_gps // 604800
    tow = toe_gps % 604800
    eph.toe = gpst2time(week, tow)
    eph.tof = gpst2time(week, tow)
    
    eph.pos[0] = float(eph_data['X'].values) * 1e3
    eph.pos[1] = float(eph_data['Y'].values) * 1e3
    eph.pos[2] = float(eph_data['Z'].values) * 1e3
    eph.vel[0] = float(eph_data['dX'].values) * 1e3
    eph.vel[1] = float(eph_data['dY'].values) * 1e3
    eph.vel[2] = float(eph_data['dZ'].values) * 1e3
    eph.acc[0] = float(eph_data['dX2'].values) * 1e3
    eph.acc[1] = float(eph_data['dY2'].values) * 1e3
    eph.acc[2] = float(eph_data['dZ2'].values) * 1e3
    
    eph.taun = float(eph_data['SVclockBias'].values)
    eph.gamn = float(eph_data['SVrelFreqBias'].values)
    
    return eph

def get_gps_time(dt):
    gps_epoch = np.datetime64('1980-01-06T00:00:00')
    return int((dt - gps_epoch) / np.timedelta64(1, 's'))

def compute_position(eph, gps_sec):
    time = gtime_t()
    time.time = GPS_TO_UNIX_OFFSET + gps_sec
    time.sec = 0.0
    
    rs = Arr1Ddouble(3)
    dts = Arr1Ddouble(1)
    var = Arr1Ddouble(1)
    
    geph2pos(time, eph, rs, dts, var)
    if rs[0] == 0.0:
        return None
    
    dt = 1E-3
    time2 = gtime_t()
    time2.time = GPS_TO_UNIX_OFFSET + gps_sec
    time2.sec = dt
    rs2 = Arr1Ddouble(3)
    dts2 = Arr1Ddouble(1)
    var2 = Arr1Ddouble(1)
    geph2pos(time2, eph, rs2, dts2, var2)
    
    return {
        'x': rs[0], 'y': rs[1], 'z': rs[2],
        'x2': rs2[0], 'y2': rs2[1], 'z2': rs2[2],
        'vx': (rs2[0] - rs[0]) / dt,
        'vy': (rs2[1] - rs[1]) / dt,
        'vz': (rs2[2] - rs[2]) / dt,
        'clock_bias': dts[0], 'clock_drift': (dts2[0] - dts[0]) / dt
    }

def generate_tests(eph, toe_gps_sec):
    tests = []
    for i in range(NUM_TESTS):
        if NUM_TESTS > 1:
            offset = int(-VALIDITY_PERIOD/2) + int(i * VALIDITY_PERIOD / (NUM_TESTS - 1))
        else:
            offset = 0
        test_gps_sec = toe_gps_sec + offset
        
        time = gtime_t()
        time.time = GPS_TO_UNIX_OFFSET + test_gps_sec
        time.sec = 0.0
        utc_time = gpst2utc(time)
        time_str_arr = Arr1Dchar(64)
        time2str(utc_time, time_str_arr, 3)
        time_str = ''.join([c for c in list(time_str_arr) if ord(c) != 0])
        
        pos = compute_position(eph, test_gps_sec)
        if pos:
            tests.append([test_gps_sec, offset, time_str, pos['x'], pos['y'], pos['z'],
                         pos['x2'], pos['y2'], pos['z2'], pos['vx'], pos['vy'], pos['vz'],
                         pos['clock_bias'], pos['clock_drift']])
    
    return tests

nav = gr.load('brdc_nav.rnx', use='R')

for time in nav.time.values:
    gps_sec = get_gps_time(time)
    ephemerides = []
    
    for sv in nav.sv.values:
        if str(sv)[0] != 'R':
            continue
        
        try:
            eph_data = nav.sel(sv=sv, time=time)
            if np.isnan(eph_data['X'].values):
                continue
            
            slot = int(str(sv)[1:].split('_')[0])
            toe_gps_sec = gps_sec
            
            eph = build_glo_eph(eph_data, slot)
            tests = generate_tests(eph, toe_gps_sec)
            if not tests:
                continue
            
            glo_timestamp = gps_to_glo_timestamp(toe_gps_sec)
            
            eph_array = [
                slot,
                int(eph_data['FreqNum'].values),
                [glo_timestamp],
                [float(eph_data['X'].values), float(eph_data['Y'].values), float(eph_data['Z'].values)],
                [float(eph_data['dX'].values), float(eph_data['dY'].values), float(eph_data['dZ'].values)],
                [float(eph_data['dX2'].values), float(eph_data['dY2'].values), float(eph_data['dZ2'].values)],
                float(eph_data['SVclockBias'].values),
                float(eph_data['SVrelFreqBias'].values),
                int(eph_data['health'].values),
                int(eph_data['AgeOpInfo'].values) if 'AgeOpInfo' in eph_data else 0,
                0,
                0
            ]
            
            ephemerides.append([eph_array, tests])
        except Exception as e:
            print(f"Error processing {sv}: {e}", file=sys.stderr)
            continue
    
    if ephemerides:
        with open(f'glo/{gps_sec}.msgpack', 'wb') as f:
            f.write(msgpack.packb(ephemerides))
        
        print(f"{gps_sec}: {len(ephemerides)} satellites")
