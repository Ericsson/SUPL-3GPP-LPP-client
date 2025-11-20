#!/usr/bin/env python3
import georinex as gr
import numpy as np
from pyrtklib import eph2pos, gpst2time, gpst2utc, time2str, eph_t, gtime_t, Arr1Ddouble, Arr1Dchar, tracelevel, traceopen
import msgpack
import warnings
import sys
warnings.filterwarnings('ignore')

traceopen('rtklib_trace.txt')
tracelevel(5)

GPS_TO_UNIX_OFFSET = 315964800
VALIDITY_PERIOD = 4 * 3600
AOD_OFFSET = 0
NUM_TESTS = 10

def get_gps_time(dt):
    gps_epoch = np.datetime64('1980-01-06T00:00:00')
    return int((dt - gps_epoch) / np.timedelta64(1, 's'))

def build_qzss_eph(eph_data, prn):
    eph = eph_t()
    
    week = int(eph_data['GPSWeek'].values)
    toe_sow = float(eph_data['Toe'].values)
    toc_gps = get_gps_time(np.datetime64(eph_data.time.values, 'ns'))
    toc_sow = toc_gps % 604800
    
    eph.sat = prn
    eph.week = week
    eph.toes = toe_sow
    eph.toe = gpst2time(week, toe_sow)
    eph.toc = gpst2time(week, toc_sow)
    eph.iode = int(eph_data['IODE'].values)
    eph.iodc = int(eph_data['IODC'].values)
    eph.sva = int(eph_data['SVacc'].values)
    eph.svh = int(eph_data['health'].values)
    eph.code = int(eph_data['CodesL2'].values)
    eph.flag = int(eph_data['L2Pflag'].values)
    eph.fit = int(eph_data['FitIntvl'].values)
    eph.tgd[0] = float(eph_data['TGD'].values)
    eph.f2 = float(eph_data['SVclockDriftRate'].values)
    eph.f1 = float(eph_data['SVclockDrift'].values)
    eph.f0 = float(eph_data['SVclockBias'].values)
    eph.A = float(eph_data['sqrtA'].values) ** 2
    eph.e = float(eph_data['Eccentricity'].values)
    eph.i0 = float(eph_data['Io'].values)
    eph.OMG0 = float(eph_data['Omega0'].values)
    eph.omg = float(eph_data['omega'].values)
    eph.M0 = float(eph_data['M0'].values)
    eph.deln = float(eph_data['DeltaN'].values)
    eph.OMGd = float(eph_data['OmegaDot'].values)
    eph.idot = float(eph_data['IDOT'].values)
    eph.crc = float(eph_data['Crc'].values)
    eph.crs = float(eph_data['Crs'].values)
    eph.cuc = float(eph_data['Cuc'].values)
    eph.cus = float(eph_data['Cus'].values)
    eph.cic = float(eph_data['Cic'].values)
    eph.cis = float(eph_data['Cis'].values)
    
    return eph

def compute_position(eph, gps_sec):
    time = gtime_t()
    time.time = GPS_TO_UNIX_OFFSET + gps_sec
    time.sec = 0.0
    
    rs = Arr1Ddouble(3)
    dts = Arr1Ddouble(1)
    var = Arr1Ddouble(1)
    
    eph2pos(time, eph, rs, dts, var)
    if rs[0] == 0.0:
        return None
    
    dt = 1E-3
    time2 = gtime_t()
    time2.time = GPS_TO_UNIX_OFFSET + gps_sec
    time2.sec = dt
    rs2 = Arr1Ddouble(3)
    dts2 = Arr1Ddouble(1)
    var2 = Arr1Ddouble(1)
    eph2pos(time2, eph, rs2, dts2, var2)
    
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
        offset = int(-VALIDITY_PERIOD/2) + AOD_OFFSET + int(i * (VALIDITY_PERIOD - AOD_OFFSET * 2) / (NUM_TESTS - 1))
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

nav = gr.load('brdc_nav.rnx', use='J')

for time in nav.time.values:
    gps_sec = get_gps_time(time)
    ephemerides = []
    
    for sv in nav.sv.values:
        if str(sv)[0] != 'J':
            continue
        
        try:
            eph_data = nav.sel(sv=sv, time=time)
            if np.isnan(eph_data['sqrtA'].values):
                continue
            
            prn = int(str(sv)[1:].split('_')[0])
            toe_sow = float(eph_data['Toe'].values)
            
            week = int(eph_data['GPSWeek'].values)
            toe_gps_sec = week * 604800 + int(toe_sow)
            
            eph = build_qzss_eph(eph_data, prn)
            tests = generate_tests(eph, toe_gps_sec)
            if not tests:
                continue
            
            toc_gps = get_gps_time(np.datetime64(eph_data.time.values, 'ns'))
            
            eph_array = [
                prn, week, int(eph_data['CodesL2'].values), int(eph_data['SVacc'].values),
                int(eph_data['health'].values), 0, int(eph_data['IODC'].values),
                int(eph_data['IODE'].values), 0, float(toc_gps % 604800), toe_sow,
                float(eph_data['TGD'].values), float(eph_data['SVclockDriftRate'].values),
                float(eph_data['SVclockDrift'].values), float(eph_data['SVclockBias'].values),
                float(eph_data['Crc'].values), float(eph_data['Crs'].values),
                float(eph_data['Cuc'].values), float(eph_data['Cus'].values),
                float(eph_data['Cic'].values), float(eph_data['Cis'].values),
                float(eph_data['Eccentricity'].values), float(eph_data['M0'].values),
                float(eph_data['DeltaN'].values), float(eph_data['sqrtA'].values) ** 2,
                float(eph_data['Io'].values), float(eph_data['Omega0'].values),
                float(eph_data['omega'].values), float(eph_data['OmegaDot'].values),
                float(eph_data['IDOT'].values), int(eph_data['FitIntvl'].values) != 0,
                int(eph_data['L2Pflag'].values) != 0
            ]
            
            ephemerides.append([eph_array, tests])
        except Exception as e:
            print(f"Error processing {sv}: {e}", file=sys.stderr)
            continue
    
    if ephemerides:
        with open(f'qzss/{gps_sec}.msgpack', 'wb') as f:
            f.write(msgpack.packb(ephemerides))
        
        print(f"{gps_sec}: {len(ephemerides)} satellites")
