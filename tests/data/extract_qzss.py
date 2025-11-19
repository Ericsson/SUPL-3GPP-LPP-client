#!/usr/bin/env python3
"""Extract QZSS ephemeris test data from RINEX navigation file."""
import georinex as gr
import numpy as np
from pyrtklib import readrnx, satpos, gpst2utc, time2str, satid2no, gtime_t, nav_t, Arr1Ddouble, Arr1Dint, Arr1Dchar

GPS_TO_UNIX_OFFSET = 315964800
VALIDITY_PERIOD = 4 * 3600
AOD_OFFSET = 600
NUM_TESTS = 10

def get_gps_time(dt):
    gps_epoch = np.datetime64('1980-01-06T00:00:00')
    return int((dt - gps_epoch) / np.timedelta64(1, 's'))

def compute_position(rtklib_nav, gps_sec, prn):
    time = gtime_t()
    time.time = GPS_TO_UNIX_OFFSET + gps_sec
    time.sec = 0.0
    
    satno = satid2no(f"J{prn:02d}")
    rs = Arr1Ddouble(6)
    dts = Arr1Ddouble(2)
    var = Arr1Ddouble(1)
    svh = Arr1Dint(1)
    
    ret = satpos(time, time, satno, 0, rtklib_nav, rs, dts, var, svh)
    if ret == 0 or rs[0] == 0:
        return None
    
    return {
        'x': rs[0], 'y': rs[1], 'z': rs[2],
        'vx': rs[3], 'vy': rs[4], 'vz': rs[5],
        'clock_bias': dts[0], 'clock_drift': dts[1]
    }

def generate_tests(rtklib_nav, toe_gps_sec, prn):
    tests = []
    for i in range(NUM_TESTS):
        offset = AOD_OFFSET + int(i * (VALIDITY_PERIOD - AOD_OFFSET) / (NUM_TESTS - 1))
        test_gps_sec = toe_gps_sec + offset
        
        time = gtime_t()
        time.time = GPS_TO_UNIX_OFFSET + test_gps_sec
        time.sec = 0.0
        utc_time = gpst2utc(time)
        time_str_arr = Arr1Dchar(64)
        time2str(utc_time, time_str_arr, 3)
        time_str = ''.join([c for c in list(time_str_arr) if ord(c) != 0])
        
        pos = compute_position(rtklib_nav, test_gps_sec, prn)
        if pos:
            tests.append({'gps_sec': test_gps_sec, 'time_str': time_str, **pos})
    
    return tests

nav = gr.load('brdc_nav.rnx', use='J')
rtklib_nav = nav_t()
readrnx('brdc_nav.rnx', 1, '', None, rtklib_nav, None)

test_id = 0
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
            
            tests = generate_tests(rtklib_nav, toe_gps_sec, prn)
            if not tests:
                continue
            
            toc_gps = get_gps_time(np.datetime64(eph_data.time.values, 'ns'))
            
            eph = {
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
                'tests': tests
            }
            ephemerides.append(eph)
        except Exception as e:
            continue
    
    if ephemerides:
        with open(f'qzss/{gps_sec}.txt', 'w') as f:
            f.write("# QZSS Ephemeris Test Data\n")
            f.write("# Format: EPH prn week toe toc iode iodc tgd af2 af1 af0 crc crs cuc cus cic cis e m0 delta_n a i0 omega0 omega omega_dot idot health ura fit_flag\n")
            f.write("# Format: TEST test_id gps_sec time_str x y z vx vy vz clock_bias clock_drift\n\n")
            
            for eph in ephemerides:
                f.write(f"EPH {eph['prn']} {eph['week']} {eph['toe']} {eph['toc']} ")
                f.write(f"{eph['iode']} {eph['iodc']} {eph['tgd']} {eph['af2']} {eph['af1']} {eph['af0']} ")
                f.write(f"{eph['crc']} {eph['crs']} {eph['cuc']} {eph['cus']} {eph['cic']} {eph['cis']} ")
                f.write(f"{eph['e']} {eph['m0']} {eph['delta_n']} {eph['a']} {eph['i0']} {eph['omega0']} ")
                f.write(f"{eph['omega']} {eph['omega_dot']} {eph['idot']} {eph['health']} {eph['ura']} {eph['fit_flag']}\n")
                
                for test in eph['tests']:
                    f.write(f"TEST {test_id} {test['gps_sec']} \"{test['time_str']}\" ")
                    f.write(f"{test['x']} {test['y']} {test['z']} ")
                    f.write(f"{test['vx']} {test['vy']} {test['vz']} ")
                    f.write(f"{test['clock_bias']} {test['clock_drift']}\n")
                    test_id += 1
                f.write("\n")
        
        print(f"{gps_sec}: {len(ephemerides)} satellites")
