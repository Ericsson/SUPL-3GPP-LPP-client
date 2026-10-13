#!/usr/bin/env python3
"""
Example script to analyze IOD tracking data.

This script demonstrates how to:
1. Load the IOD tracking CSV files
2. Check for IOD consistency between SSR and ephemeris
3. Visualize IOD changes over time
4. Detect potential issues
"""

import pandas as pd
import sys
from pathlib import Path

GNSS_NAMES = {0: 'GPS', 2: 'Galileo', 3: 'BeiDou'}

def load_data(ssr_file='iod_ssr_orbit.csv', eph_file='iod_ephemeris.csv'):
    """Load IOD tracking data from CSV files."""
    try:
        ssr_data = pd.read_csv(ssr_file)
        print(f"✓ Loaded {len(ssr_data)} SSR orbit correction records")
    except FileNotFoundError:
        print(f"✗ File not found: {ssr_file}")
        ssr_data = None
    
    try:
        eph_data = pd.read_csv(eph_file)
        print(f"✓ Loaded {len(eph_data)} ephemeris records")
    except FileNotFoundError:
        print(f"✗ File not found: {eph_file}")
        eph_data = None
    
    return ssr_data, eph_data

def analyze_satellite(ssr_data, eph_data, gnss, prn):
    """Analyze IOD data for a specific satellite."""
    gnss_name = GNSS_NAMES.get(gnss, f'GNSS-{gnss}')
    print(f"\n=== Analysis for {gnss_name} PRN {prn} ===\n")
    
    # Filter data for this satellite
    if ssr_data is not None:
        sat_ssr = ssr_data[(ssr_data['gnss'] == gnss) & (ssr_data['prn'] == prn)]
        if len(sat_ssr) > 0:
            print(f"SSR Orbit Corrections: {len(sat_ssr)} records")
            print(f"  SSR IOD range: {sat_ssr['ssr_iod'].min()} - {sat_ssr['ssr_iod'].max()}")
            print(f"  Ephemeris IOD range: {sat_ssr['iod'].min()} - {sat_ssr['iod'].max()}")
            print(f"  Time span: {sat_ssr['current_tai'].min():.1f} - {sat_ssr['current_tai'].max():.1f} TAI")
            
            # Check for IOD changes
            unique_ssr_iods = sat_ssr['ssr_iod'].nunique()
            unique_iods = sat_ssr['iod'].nunique()
            print(f"  Unique SSR IODs: {unique_ssr_iods}")
            print(f"  Unique ephemeris IODs: {unique_iods}")
        else:
            print("No SSR orbit correction data found")
    
    if eph_data is not None:
        sat_eph = eph_data[(eph_data['gnss'] == gnss) & (eph_data['prn'] == prn)]
        if len(sat_eph) > 0:
            print(f"\nEphemeris Data: {len(sat_eph)} records")
            print(f"  LPP IOD range: {sat_eph['lpp_iod'].min()} - {sat_eph['lpp_iod'].max()}")
            print(f"  IODE range: {sat_eph['iode'].min()} - {sat_eph['iode'].max()}")
            if gnss == 0:  # GPS has IODC
                print(f"  IODC range: {sat_eph['iodc'].min()} - {sat_eph['iodc'].max()}")
            print(f"  TOE range: {sat_eph['toe'].min():.1f} - {sat_eph['toe'].max():.1f}")
            print(f"  Time span: {sat_eph['current_tai'].min():.1f} - {sat_eph['current_tai'].max():.1f} TAI")
            
            # Check for ephemeris updates
            unique_iodes = sat_eph['iode'].nunique()
            print(f"  Unique IODEs: {unique_iodes}")
        else:
            print("\nNo ephemeris data found")

def check_iod_consistency(ssr_data, eph_data):
    """Check if SSR IODs match ephemeris IODs."""
    print("\n=== IOD Consistency Check ===\n")
    
    if ssr_data is None or eph_data is None:
        print("Cannot perform consistency check - missing data")
        return
    
    # Group by satellite
    for gnss in ssr_data['gnss'].unique():
        gnss_name = GNSS_NAMES.get(gnss, f'GNSS-{gnss}')
        print(f"\n{gnss_name}:")
        
        for prn in ssr_data[ssr_data['gnss'] == gnss]['prn'].unique():
            sat_ssr = ssr_data[(ssr_data['gnss'] == gnss) & (ssr_data['prn'] == prn)]
            sat_eph = eph_data[(eph_data['gnss'] == gnss) & (eph_data['prn'] == prn)]
            
            if len(sat_ssr) == 0 or len(sat_eph) == 0:
                continue
            
            # Get unique IODs
            ssr_iods = set(sat_ssr['iod'].unique())
            eph_iods = set(sat_eph['iode'].unique())
            
            # Check for matches
            matching = ssr_iods & eph_iods
            ssr_only = ssr_iods - eph_iods
            eph_only = eph_iods - ssr_iods
            
            status = "✓" if len(ssr_only) == 0 else "⚠"
            print(f"  PRN {prn:2d}: {status} SSR IODs: {len(ssr_iods)}, Eph IODs: {len(eph_iods)}, Matching: {len(matching)}")
            
            if len(ssr_only) > 0:
                print(f"           SSR IODs without ephemeris: {sorted(ssr_only)}")
            if len(eph_only) > 0:
                print(f"           Ephemeris IODs without SSR: {sorted(eph_only)}")

def summary_statistics(ssr_data, eph_data):
    """Print summary statistics."""
    print("\n=== Summary Statistics ===\n")
    
    if ssr_data is not None:
        print("SSR Orbit Corrections:")
        for gnss in sorted(ssr_data['gnss'].unique()):
            gnss_name = GNSS_NAMES.get(gnss, f'GNSS-{gnss}')
            gnss_data = ssr_data[ssr_data['gnss'] == gnss]
            print(f"  {gnss_name}: {len(gnss_data)} records, {gnss_data['prn'].nunique()} satellites")
    
    if eph_data is not None:
        print("\nEphemeris Data:")
        for gnss in sorted(eph_data['gnss'].unique()):
            gnss_name = GNSS_NAMES.get(gnss, f'GNSS-{gnss}')
            gnss_data = eph_data[eph_data['gnss'] == gnss]
            print(f"  {gnss_name}: {len(gnss_data)} records, {gnss_data['prn'].nunique()} satellites")

def main():
    print("=== IOD Tracking Data Analysis ===\n")
    
    # Load data
    ssr_data, eph_data = load_data()
    
    if ssr_data is None and eph_data is None:
        print("\n✗ No data files found. Please run example-client first.")
        return 1
    
    # Summary statistics
    summary_statistics(ssr_data, eph_data)
    
    # Check consistency
    check_iod_consistency(ssr_data, eph_data)
    
    # Example: Analyze specific satellite (GPS PRN 1)
    if ssr_data is not None or eph_data is not None:
        # Find first available satellite
        if ssr_data is not None and len(ssr_data) > 0:
            first_gnss = ssr_data.iloc[0]['gnss']
            first_prn = ssr_data.iloc[0]['prn']
        elif eph_data is not None and len(eph_data) > 0:
            first_gnss = eph_data.iloc[0]['gnss']
            first_prn = eph_data.iloc[0]['prn']
        else:
            return 0
        
        analyze_satellite(ssr_data, eph_data, int(first_gnss), int(first_prn))
    
    print("\n✓ Analysis complete!")
    return 0

if __name__ == '__main__':
    sys.exit(main())
