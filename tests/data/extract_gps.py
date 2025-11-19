#!/usr/bin/env python3
"""
Extract GPS ephemeris test data from RINEX navigation file.

Uses eph2pos with manually built ephemeris to avoid RTKLIB's seleph() issues.
Filters out off-nominal TOE values (not on 2-hour boundaries).
"""
import georinex as gr
import numpy as np
from pyrtklib import eph2pos, gpst2time, gpst2utc, time2str, eph_t, gtime_t, Arr1Ddouble, Arr1Dchar, tracelevel, traceopen
import warnings
import sys
warnings.filterwarnings('ignore')

# Enable RTKLIB tracing
traceopen('rtklib_trace.txt')
tracelevel(5)

GPS_TO_UNIX_OFFSET = 315964800
VALIDITY_PERIOD = 4 * 3600  # 4 hours
AOD_OFFSET = 0
NUM_TESTS = 10

def get_gps_time(dt):
    gps_epoch = np.datetime64('1980-01-06T00:00:00')
    return int((dt - gps_epoch) / np.timedelta64(1, 's'))

def build_gps_eph(eph_data, prn):
    """Build RTKLIB eph_t from georinex data"""
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
    
    # Compute velocity by finite difference
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
            tests.append({'gps_sec': test_gps_sec, 'offset': offset, 'time_str': time_str, **pos})
    
    return tests

nav = gr.load('brdc_nav.rnx', use='G')

test_id = 0
for time in nav.time.values:
    gps_sec = get_gps_time(time)
    ephemerides = []
    
    for sv in nav.sv.values:
        if str(sv)[0] != 'G':
            continue
        
        try:
            eph_data = nav.sel(sv=sv, time=time)
            if np.isnan(eph_data['sqrtA'].values):
                continue
            
            prn = int(str(sv)[1:].split('_')[0])
            toe_sow = float(eph_data['Toe'].values)
            
            week = int(eph_data['GPSWeek'].values)
            toe_gps_sec = week * 604800 + int(toe_sow)
            
            eph = build_gps_eph(eph_data, prn)
            tests = generate_tests(eph, toe_gps_sec)
            if not tests:
                continue
            
            toc_gps = get_gps_time(np.datetime64(eph_data.time.values, 'ns'))
            
            eph_dict = {
                'prn': prn, 'week': week, 'toe': toe_sow, 'toc': float(toc_gps % 604800),
                'iode': int(eph_data['IODE'].values), 'iodc': int(eph_data['IODC'].values),
                'tgd': float(eph_data['TGD'].values),
                'af2': float(eph_data['SVclockDriftRate'].values),
                'af1': float(eph_data['SVclockDrift'].values),
                'af0': float(eph_data['SVclockBias'].values),
                'crc': float(eph_data['Crc'].values), 'crs': float(eph_data['Crs'].values),
                'cuc': float(eph_data['Cuc'].values), 'cus': float(eph_data['Cus'].values),
                'cic': float(eph_data['Cic'].values), 'cis': float(eph_data['Cis'].values),
                'e': float(eph_data['Eccentricity'].values), 'm0': float(eph_data['M0'].values),
                'delta_n': float(eph_data['DeltaN'].values),
                'a': float(eph_data['sqrtA'].values) ** 2,
                'i0': float(eph_data['Io'].values), 'omega0': float(eph_data['Omega0'].values),
                'omega': float(eph_data['omega'].values),
                'omega_dot': float(eph_data['OmegaDot'].values),
                'idot': float(eph_data['IDOT'].values),
                'health': int(eph_data['health'].values), 'ura': int(eph_data['SVacc'].values),
                'fit_flag': int(eph_data['FitIntvl'].values),
                'l2p_flag': int(eph_data['L2Pflag'].values),
                'l2_code': int(eph_data['CodesL2'].values),
                'tests': tests
            }
            ephemerides.append(eph_dict)
        except Exception as e:
            print(f"Error processing {sv}: {e}", file=sys.stderr)
            continue
    
    if ephemerides:
        with open(f'gps/{gps_sec}.txt', 'w') as f:
            f.write("# GPS Ephemeris Test Data\n")
            f.write("# Format: EPH prn week toe toc iode iodc tgd af2 af1 af0 crc crs cuc cus cic cis e m0 delta_n a i0 omega0 omega omega_dot idot health ura fit_flag l2p_flag l2_code\n")
            f.write("# Format: TEST test_id gps_sec offset time_str x y z vx vy vz clock_bias clock_drift\n\n")
            
            for eph in ephemerides:
                f.write(f"EPH {eph['prn']} {eph['week']} {eph['toe']} {eph['toc']} ")
                f.write(f"{eph['iode']} {eph['iodc']} {eph['tgd']} {eph['af2']} {eph['af1']} {eph['af0']} ")
                f.write(f"{eph['crc']} {eph['crs']} {eph['cuc']} {eph['cus']} {eph['cic']} {eph['cis']} ")
                f.write(f"{eph['e']} {eph['m0']} {eph['delta_n']} {eph['a']} {eph['i0']} {eph['omega0']} ")
                f.write(f"{eph['omega']} {eph['omega_dot']} {eph['idot']} {eph['health']} {eph['ura']} ")
                f.write(f"{eph['fit_flag']} {eph['l2p_flag']} {eph['l2_code']}\n")
                
                for test in eph['tests']:
                    f.write(f"TEST {test_id} {test['gps_sec']} {test['offset']} \"{test['time_str']}\" ")
                    f.write(f"{test['x']} {test['y']} {test['z']} ")
                    f.write(f"{test['vx']} {test['vy']} {test['vz']} ")
                    f.write(f"{test['clock_bias']} {test['clock_drift']}\n")
                    test_id += 1
                f.write("\n")
        
        print(f"{gps_sec}: {len(ephemerides)} satellites")
