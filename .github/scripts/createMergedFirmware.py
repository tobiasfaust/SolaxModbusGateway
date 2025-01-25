'''
Description:
This script merges firmware binaries for ESP32 and ESP8266 chip families using esptool.py.
It reads partition offsets from the partition CSV file and constructs the appropriate merge_bin command.
Usage:
    python createMergedFirmware.py -c <ChipFamily> -b <BuildDir> -p <PathOfPartitionsCSV>
Arguments:
    -c, --ChipFamily (str): The chip family (e.g., ESP8266, ESP32-S3, etc.). Default is 'ESP32'. Required.
    -b, --BuildDir (str): The build directory containing the built firmware binaries. Required.
    -p, --PathOfPartitionsCSV (str): The path to the partitions.csv file. Default is 'partitions.csv'. Required.
Functions:
    readOffsetFromPartitionCSV(path: str, name: str) -> int:
        Args:
        Returns:
Example:
    python createMergedFirmware.py -c ESP32 -b ./build -p ./partitions.csv
'''

from myUtils import *
import argparse
import os

parser = argparse.ArgumentParser()

# erwartete Parameter
parser.add_argument('-c', '--ChipFamily', type=str, help='chipFamily, ESP8266, ESP32-S3, ...', default='ESP32', required=True)
parser.add_argument('-b', '--BuildDir', type=str, help='Build Verzeichnis welches die fertig gebauten Firmwares enthält', required=True)
parser.add_argument('-p', '--PathOfPartitionsCSV', type=str, help='Pfad wo die partitions.csv liegt', default='partitions.csv', required=True)

# Parsen der Argumente
args = parser.parse_args()
result = None

# Bootloader offsets vary by chip
bootloader_offsets = {
    "ESP32": "0x1000",
    "ESP32-S2": "0x1000",
    "ESP32-S3": "0x0",
    "ESP32-C2": "0x0",
    "ESP32-C3": "0x0",
    "ESP32-C5": "0x2000",
    "ESP32-C6": "0x0",
    "ESP32-H2": "0x0",
    "ESP32-P4": "0x2000",
}

if(not os.path.isfile(args.PathOfPartitionsCSV)):
    logging.error(f'File {args.PathOfPartitionsCSV} does not exist')
    exit(1)

if args.BuildDir and os.path.isdir(args.BuildDir):
    if 'ESP32' in args.ChipFamily:
        bootloader_offset = bootloader_offsets[args.ChipFamily]

        result = f'esptool.py --chip {args.ChipFamily} merge_bin \
            --output {args.BuildDir}/merged-firmware.bin \
            --flash_mode dout \
            --flash_freq 80m \
            --flash_size 4MB \
            {bootloader_offset} {args.BuildDir}/bootloader.bin \
            0x8000 {args.BuildDir}/partitions.bin \
            {readOffsetFromPartitionCSV("partitions.csv", "app0")} {args.BuildDir}/firmware.bin'
        
        if os.path.isfile(f'{args.BuildDir}/littlefs.bin'):
            result += f' {readOffsetFromPartitionCSV("partitions.csv", "spiffs")} {args.BuildDir}/littlefs.bin'

    elif 'ESP8266' in args.ChipFamily and os.path.isfile(f'{args.BuildDir}/littlefs.bin'):
        result = f'esptool.py --chip {args.ChipFamily} merge_bin \
            --output {args.BuildDir}/merged-firmware.bin \
            --flash_mode dout \
            --flash_freq 40m \
            --flash_size 4MB \
            0x0000 {args.BuildDir}/firmware.bin \
            {readOffsetFromPartitionCSV("partitions.csv", "spiffs")} {args.BuildDir}/littlefs.bin'

print(f'command={result}')