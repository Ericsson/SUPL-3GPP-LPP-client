#!/usr/bin/env python3
import sys
import os
from osgeo import gdal
import numpy as np

def determine_precision(data):
    abs_max = np.max(np.abs(data))
    if abs_max < 10:
        return 3
    elif abs_max < 100:
        return 2
    else:
        return 1

def geotiff_to_cpp(input_file, output_dir, model_name):
    dataset = gdal.Open(input_file)
    if not dataset:
        print(f"Error: Could not open {input_file}")
        sys.exit(1)
    
    num_bands = dataset.RasterCount
    if num_bands != 3:
        print(f"Error: Expected 3 bands (E, N, U), found {num_bands}")
        sys.exit(1)
    
    # Read bands by name
    bands = {}
    for i in range(1, num_bands + 1):
        band = dataset.GetRasterBand(i)
        desc = band.GetDescription().lower()
        if 'east' in desc:
            bands['east'] = band.ReadAsArray()
        elif 'north' in desc:
            bands['north'] = band.ReadAsArray()
        elif 'up' in desc:
            bands['up'] = band.ReadAsArray()
    
    if len(bands) != 3:
        print(f"Error: Could not identify all three bands (east, north, up)")
        sys.exit(1)
    
    east_data = bands['east']
    north_data = bands['north']
    up_data = bands['up']
    
    geotransform = dataset.GetGeoTransform()
    rows, cols = east_data.shape
    origin_lon = geotransform[0]
    origin_lat = geotransform[3]
    pixel_width = geotransform[1]
    pixel_height = geotransform[5]
    
    precision = max(determine_precision(east_data), 
                   determine_precision(north_data),
                   determine_precision(up_data))
    
    hpp_file = os.path.join(output_dir, 'include', 'coordinates', 'model', f'{model_name}.hpp')
    cpp_file = os.path.join(output_dir, 'src', 'model', f'{model_name}.cpp')
    
    os.makedirs(os.path.dirname(hpp_file), exist_ok=True)
    os.makedirs(os.path.dirname(cpp_file), exist_ok=True)
    
    with open(hpp_file, 'w') as f:
        f.write('#pragma once\n')
        f.write('#include "coordinates/enu.hpp"\n')
        f.write('#include "coordinates/llh.hpp"\n\n')
        f.write('namespace coordinates {\n')
        f.write(f'namespace {model_name} {{\n\n')
        f.write('Enu<> lookup(Llh<> const& position);\n\n')
        f.write(f'}} // namespace {model_name}\n')
        f.write('} // namespace coordinates\n')
    
    with open(cpp_file, 'w') as f:
        f.write(f'#include "coordinates/model/{model_name}.hpp"\n')
        f.write('#include <cmath>\n\n')
        f.write('namespace coordinates {\n')
        f.write(f'namespace {model_name} {{\n\n')
        
        f.write(f'constexpr int ROWS = {rows};\n')
        f.write(f'constexpr int COLS = {cols};\n')
        f.write(f'constexpr double ORIGIN_LON = {origin_lon};\n')
        f.write(f'constexpr double ORIGIN_LAT = {origin_lat};\n')
        f.write(f'constexpr double PIXEL_WIDTH = {pixel_width};\n')
        f.write(f'constexpr double PIXEL_HEIGHT = {pixel_height};\n\n')
        
        f.write('// clang-format off\n')
        for name, data in [('EAST', east_data), ('NORTH', north_data), ('UP', up_data)]:
            f.write(f'constexpr float {name}_DATA[{rows}][{cols}] = {{\n')
            for i in range(rows):
                f.write('    {')
                f.write(', '.join(f'{data[i,j]:.{precision}f}' for j in range(cols)))
                f.write('},\n')
            f.write('};\n\n')
        f.write('// clang-format on\n\n')
        
        f.write('static double bilinear_interp(const float data[ROWS][COLS], int x0, int y0, double fx, double fy) {\n')
        f.write('    double v00 = data[y0][x0];\n')
        f.write('    double v10 = data[y0][x0 + 1];\n')
        f.write('    double v01 = data[y0 + 1][x0];\n')
        f.write('    double v11 = data[y0 + 1][x0 + 1];\n')
        f.write('    return v00 * (1.0 - fx) * (1.0 - fy) + v10 * fx * (1.0 - fy) + v01 * (1.0 - fx) * fy + v11 * fx * fy;\n')
        f.write('}\n\n')
        
        f.write('Enu<> lookup(Llh<> const& position) {\n')
        f.write('    double lon = position.longitude_deg();\n')
        f.write('    double lat = position.latitude_deg();\n')
        f.write('    double x = (lon - ORIGIN_LON) / PIXEL_WIDTH;\n')
        f.write('    double y = (lat - ORIGIN_LAT) / PIXEL_HEIGHT;\n')
        f.write('    if (x < 0.0 || x >= COLS - 1 || y < 0.0 || y >= ROWS - 1) {\n')
        f.write('        return Enu<>{Vector3d::Zero()};\n')
        f.write('    }\n')
        f.write('    int x0 = static_cast<int>(x);\n')
        f.write('    int y0 = static_cast<int>(y);\n')
        f.write('    double fx = x - x0;\n')
        f.write('    double fy = y - y0;\n')
        f.write('    double e = bilinear_interp(EAST_DATA, x0, y0, fx, fy) * 0.001;\n')
        f.write('    double n = bilinear_interp(NORTH_DATA, x0, y0, fx, fy) * 0.001;\n')
        f.write('    double u = bilinear_interp(UP_DATA, x0, y0, fx, fy) * 0.001;\n')
        f.write('    return Enu<>{Vector3d(e, n, u)};\n')
        f.write('}\n\n')
        f.write(f'}} // namespace {model_name}\n')
        f.write('} // namespace coordinates\n')
    
    print(f"Generated {hpp_file} and {cpp_file}")
    print(f"Grid size: {rows}x{cols}")
    print(f"Coverage: lon [{origin_lon:.2f}, {origin_lon + cols*pixel_width:.2f}], lat [{origin_lat + rows*pixel_height:.2f}, {origin_lat:.2f}]")
    print(f"Precision: {precision} decimal places")
    print(f"Units: m/year (ENU) - converted from mm/year in source")

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("Usage: geotiff_to_cpp.py <input.tif> <output_dir> <model_name>")
        sys.exit(1)
    
    geotiff_to_cpp(sys.argv[1], sys.argv[2], sys.argv[3])
