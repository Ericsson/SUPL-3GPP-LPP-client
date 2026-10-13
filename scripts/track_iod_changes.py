#!/usr/bin/env python3
import csv
import sys

def parse_csv(filename):
    data = []
    try:
        with open(filename, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                data.append(row)
    except FileNotFoundError:
        return None
    return data

def format_time(tai_seconds):
    hours = int(tai_seconds / 3600) % 24
    minutes = int((tai_seconds % 3600) / 60)
    seconds = tai_seconds % 60
    return f"{hours:02d}:{minutes:02d}:{seconds:06.3f}"

def track_iod_changes(gnss, prn):
    ssr_data = parse_csv('iod_ssr_orbit.csv')
    eph_data = parse_csv('iod_ephemeris.csv')
    
    if not ssr_data:
        print("Error: iod_ssr_orbit.csv not found")
        return
    if not eph_data:
        print("Error: iod_ephemeris.csv not found")
        return
    
    gnss_names = {0: 'GPS', 2: 'Galileo', 3: 'BeiDou'}
    ssr_gnss = gnss + 1
    ssr_gnss_str = str(ssr_gnss)
    gnss_str = str(gnss)
    prn_str = str(prn)
    
    ssr_records = [r for r in ssr_data if r['gnss'] == ssr_gnss_str and r['prn'] == prn_str]
    eph_records = [r for r in eph_data if r['gnss'] == gnss_str and r['prn'] == prn_str]
    
    if not ssr_records and not eph_records:
        print(f"No data found for SSR GNSS {ssr_gnss} PRN {prn}")
        return
    
    print(f"\n{'='*80}")
    print(f"IOD Change Tracking: GNSS {gnss} ({gnss_names.get(gnss, 'Unknown')}) PRN {prn}")
    print(f"  SSR GNSS ID: {ssr_gnss}, Ephemeris GNSS ID: {gnss}")
    print(f"{'='*80}\n")
    
    # Build timeline
    all_times = []
    for r in ssr_records:
        all_times.append(float(r['current_tai']))
    for r in eph_records:
        all_times.append(float(r['current_tai']))
    
    if not all_times:
        print("No data")
        return
    
    min_time = min(all_times)
    max_time = max(all_times)
    
    # Create 5-second buckets
    buckets = {}
    bucket_size = 5.0
    
    for r in ssr_records:
        t = float(r['current_tai'])
        bucket = int((t - min_time) / bucket_size)
        if bucket not in buckets:
            buckets[bucket] = {'ssr': None, 'eph': None}
        buckets[bucket]['ssr'] = int(r['iod'])
    
    for r in eph_records:
        t = float(r['current_tai'])
        bucket = int((t - min_time) / bucket_size)
        if bucket not in buckets:
            buckets[bucket] = {'ssr': None, 'eph': None}
        buckets[bucket]['eph'] = int(r['lpp_iod'])
    
    # Fill gaps with previous values
    max_bucket = int((max_time - min_time) / bucket_size)
    last_ssr = None
    last_eph = None
    
    for i in range(max_bucket + 1):
        if i not in buckets:
            buckets[i] = {'ssr': last_ssr, 'eph': last_eph}
        else:
            if buckets[i]['ssr'] is not None:
                last_ssr = buckets[i]['ssr']
            else:
                buckets[i]['ssr'] = last_ssr
            
            if buckets[i]['eph'] is not None:
                last_eph = buckets[i]['eph']
            else:
                buckets[i]['eph'] = last_eph
    
    # Compress timeline
    print("Timeline (5s buckets):\n")
    print("Time          | SSR IOD | Eph IOD")
    print("-" * 40)
    
    prev_ssr = None
    prev_eph = None
    span_start = None
    
    for i in sorted(buckets.keys()):
        t = min_time + i * bucket_size
        ssr = buckets[i]['ssr']
        eph = buckets[i]['eph']
        
        if ssr != prev_ssr or eph != prev_eph:
            if span_start is not None and (prev_ssr != ssr or prev_eph != eph):
                # Print end of span if different
                if i - 1 != span_start:
                    t_end = min_time + (i - 1) * bucket_size
                    print(f"{format_time(t_end)} | {prev_ssr if prev_ssr is not None else '---':7} | {prev_eph if prev_eph is not None else '---':7}")
            
            # Print start of new span
            print(f"{format_time(t)} | {ssr if ssr is not None else '---':7} | {eph if eph is not None else '---':7}")
            span_start = i
            prev_ssr = ssr
            prev_eph = eph
    
    # Print final span end
    if span_start is not None and span_start != max_bucket:
        t_end = min_time + max_bucket * bucket_size
        print(f"{format_time(t_end)} | {prev_ssr if prev_ssr is not None else '---':7} | {prev_eph if prev_eph is not None else '---':7}")
    
    print()

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: track_iod_changes.py <gnss> <prn>")
        print("  gnss: 0=GPS, 2=Galileo, 3=BeiDou")
        print("  prn: satellite PRN number")
        sys.exit(1)
    
    gnss = int(sys.argv[1])
    prn = int(sys.argv[2])
    track_iod_changes(gnss, prn)
