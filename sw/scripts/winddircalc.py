#!/usr/bin/python3
#
# Script to generate lookup table for wind direction sensor
# using reference voltage and resistance
#

import argparse

parser = argparse.ArgumentParser()

parser.add_argument("--vdd", default=3.3, type=float, help="Supply voltage")
parser.add_argument("--r_pu", default=4700, type=int, help="Pull-up resistance")
parser.add_argument(
    "--r_s", default=2000, type=int, help="Additional series resistance"
)

args = parser.parse_args()

VDD = args.vdd
R_PU = args.r_pu
R_S = args.r_s

# Direction (degrees), Resistance (ohms), Ideal Voltage, Midpoint Voltage, Enum name
DIRS = [
    [0, 33e3, 0, 0, "WIND_N"],
    [22.5, 6.57e3, 0, 0, "WIND_NNE"],
    [45, 8.2e3, 0, 0, "WIND_NE"],
    [67.5, 891, 0, 0, "WIND_ENE"],
    [90, 1e3, 0, 0, "WIND_E"],
    [112.5, 688, 0, 0, "WIND_ESE"],
    [135, 2.2e3, 0, 0, "WIND_SE"],
    [157.5, 1.41e3, 0, 0, "WIND_SSE"],
    [180, 3.9e3, 0, 0, "WIND_S"],
    [202.5, 3.14e3, 0, 0, "WIND_SSW"],
    [225, 16e3, 0, 0, "WIND_SW"],
    [247.5, 14.12e3, 0, 0, "WIND_WSW"],
    [270, 120e3, 0, 0, "WIND_W"],
    [292.5, 42.12e3, 0, 0, "WIND_WNW"],
    [315, 64.9e3, 0, 0, "WIND_NW"],
    [337.5, 21.88e3, 0, 0, "WIND_NNW"],
]

for direction in DIRS:
    v_wind = VDD * (direction[1] + R_S) / ((direction[1] + R_S) + R_PU)
    direction[2] = round(v_wind, 3)

# Sort by voltage
sd = sorted(DIRS, key=lambda x: x[2])

# Calculate midpoint between adjacent voltages
for i in range(len(sd) - 1):
    diff = round(sd[i + 1][2] - sd[i][2], 3)
    sd[i][3] = round(sd[i][2] + diff / 2, 3)
# Add arbitrary voltage to last one
sd[-1][3] = round(sd[-1][2] + VDD / 16, 3)

print(
    """
// Wind direction shortcodes
typedef enum {
    WIND_N      = 0,
    WIND_NNE    = 1,
    WIND_NE     = 2,
    WIND_ENE    = 3,
    WIND_E      = 4,
    WIND_ESE    = 5,
    WIND_SE     = 6,
    WIND_SSE    = 7,
    WIND_S      = 8,
    WIND_SSW    = 9,
    WIND_SW     = 10,
    WIND_WSW    = 11,            
    WIND_W      = 12,
    WIND_WNW    = 13,
    WIND_NW     = 14,
    WIND_NNW    = 15,
} wind_dir_t;

typedef struct {
    uint16_t voltage;
    uint16_t degrees;
    wind_dir_t direction;
} wind_dir_lut_t;
"""
)

print("// Auto-generated with winddircalc.py")
print("// VDD{:1.2f}= R_PU={} R_S={}".format(VDD, R_PU, R_S))
print("static const wind_dir_t wind_dir_lut[] = {")
for direction in sd:
    print("    {{{:4d}, {:4d}, {}}},".format(int(direction[3] * 1000), int(direction[0] * 10), direction[4]))
print("    {   0,    0, WIND_N}};")
