import os, shutil, sys, json, argparse, logging
from myUtils import *

# Konfigurieren des Loggings
logging.basicConfig(level=logging.INFO)

# Argument parser
parser = argparse.ArgumentParser(description='Generate JSON and handle binaries.')
parser.add_argument('-r', '--repository', type=str, help='Repository name')
parser.add_argument('-s', '--build', type=str, help='Buildnummer der GithubAction (Unique ID)')
parser.add_argument('-t', '--stage', type=str, help='Stage (Branch name)')
parser.add_argument('-bp', '--binarypath', type=str, help='Path of binary files')
parser.add_argument('-fn', '--firmwarename', type=str, help='Name of the firmware')
parser.add_argument('-rp', '--releasepath', type=str, default="release", help='Path of destination, BIN and JSON files')
parser.add_argument('-rf', '--releasefile', type=str, help='Path of release file, contains version number')
parser.add_argument('-a', '--arch', type=str, help='Architecture (ESP8266|ESP32|ESP32-S2|ESP32-C3|...)')
parser.add_argument('-v', '--variant', type=str, default='standard', help='Variant (default=standard)')
parser.add_argument('-ap', '--artifactpath', type=str, default="artifacts", help='Path of all artifacts')
parser.add_argument('-d', '--debug', type=bool, help='Enable debug messages')

args = parser.parse_args()

# Echo input parameter
if args.debug:
    logging.info(f"\n\nEcho input parameter")
    logging.info(f"REPOSITORYNAME={args.repository}")
    logging.info(f"BUILDNUMMER={args.build}")
    logging.info(f"STAGE={args.stage}")
    logging.info(f"BINARYPATH={args.binarypath}")
    logging.info(f"FIRMWARENAME={args.firmwarename}")
    logging.info(f"RELEASEPATH={args.releasepath}")
    logging.info(f"RELEASEFILE={args.releasefile}")
    logging.info(f"ARCHITECTURE={args.arch}")
    logging.info(f"VARIANT={args.variant}")
    logging.info(f"ARTIFACTPATH={args.artifactpath}")
    logging.info(f"DEBUG={args.debug}")

if args.binarypath is not None and not os.path.isdir(args.binarypath):
    logging.error(f"\n\nBinarypath {args.binarypath} not found\n")
    sys.exit()

if args.releasefile is not None and not os.path.isfile(args.releasefile):
    logging.error(f"\n\nReleasefile {args.releasefile} not found\n")
    sys.exit()

os.makedirs(args.releasepath, exist_ok=True)
os.makedirs(args.artifactpath, exist_ok=True)

# Read version number from release file
with open(args.releasefile, 'r') as file:
    #ermittle den String aus der Datei
    VERSION = file.read().split('"')[1]
    VersionNumber = ''.join(filter(str.isdigit, VERSION))

FileExtension = f'{args.arch}.v{VERSION}-{args.build}.{args.variant}.{args.stage}'

# process the firmware binaries into RELEASEPATH
for root, _, files in os.walk(args.binarypath):
    for file in files:
        if file == 'firmware.bin':
            FILENAME = os.path.splitext(file)[0]
            FILEEXT = os.path.splitext(file)[1][1:]            
            DOWNLOADURL = f"https://tobiasfaust.github.io/{args.repository}/firmware/{FILENAME}.{FileExtension}.{FILEEXT}"

            ################ Create custom json ##################
            json_data = {
                "name": f"{args.repository} (v{VERSION}-{args.stage})",
                "version": VERSION,
                "variant": args.variant,
                "build": (int(args.build)),
                "number": int(f'{VersionNumber}{args.build}'),
                "stage": args.stage,
                "arch": args.arch,
                "download-url": DOWNLOADURL
            }

            if args.debug:
                logging.info(f"\n\nEcho json string")
                logging.info(json.dumps(json_data, indent=2))

            save_results_to_json(json_data, os.path.join(args.releasepath, f"{FILENAME}.{FileExtension}.json"))   
            shutil.copyfile(os.path.join(root, file), os.path.join(args.releasepath, f'{FILENAME}.{FileExtension}.{FILEEXT}'))
            
            ################ Create Manifest.json and Files.json ##################
            manifest_data = {
                "name": f"{args.repository} (v{VERSION}-{args.stage})",
                "chipFamily": args.arch,
                "stage": args.stage,
                "build": int(args.build),
                "variant": args.variant,
                "version": f"v{VERSION}",
                "parts": []
            }

            files_data = json.loads(json.dumps(manifest_data))

            SubDir = f'v{VERSION}-{args.build}-{args.stage}'

            if os.path.isfile(os.path.join(args.binarypath, "merged-firmware.bin")):
                manifest_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/{args.repository}/firmware/{SubDir}/{args.firmwarename}/merged-firmware.{FileExtension}.bin",
                    "offset": 0
                })
            else:
                # fuer ESP8266
                manifest_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/{args.repository}/firmware/{SubDir}/{args.firmwarename}/{FILENAME}.{FileExtension}.{FILEEXT}",
                    "offset": 0
                })

            # process files.json
            if os.path.isfile(os.path.join(args.binarypath, "bootloader.bin")):
                files_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/{args.repository}/firmware/{SubDir}/{args.firmwarename}/bootloader.{FileExtension}.bin",
                    "offset": 4096,
                    "filetype": "bootloader"
                })
            if os.path.isfile(os.path.join(args.binarypath, "partitions.bin")):
                files_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/{args.repository}/firmware/{SubDir}/{args.firmwarename}/partitions.{FileExtension}.bin",
                    "offset": 32768,
                    "filetype": "partitions"
                })
            if os.path.isfile(os.path.join(args.binarypath, "littlefs.bin")):
                files_data["parts"].append({
                    "path": f"https://tobiasfaust.github.io/{args.repository}/firmware/{SubDir}/{args.firmwarename}/littlefs.{FileExtension}.bin",
                    "offset": int(readOffsetFromPartitionCSV("partitions.csv", "webdata"), 16),
                    "filetype": "filesystem"
                })


            OFFSET = 0 if "ESP8266" in args.arch else int(readOffsetFromPartitionCSV("partitions.csv", "app0"), 16)
            files_data["parts"].append({
                "path": f"https://tobiasfaust.github.io/{args.repository}/firmware/{SubDir}/{args.firmwarename}/{FILENAME}.{FileExtension}.{FILEEXT}",
                "offset": OFFSET,
                "filetype": "firmware"
            })

            if args.debug:
                logging.info(f"\n\nEcho Manifest string")
                logging.info(json.dumps(manifest_data, indent=2))

                logging.info(f"\n\nEcho Files string")
                logging.info(json.dumps(files_data, indent=2))

            save_results_to_json(manifest_data, os.path.join(args.releasepath, "manifest.json"))
            save_results_to_json(manifest_data, os.path.join(args.artifactpath, "manifest.json"))

            save_results_to_json(files_data, os.path.join(args.releasepath, "files.json"))
            save_results_to_json(files_data, os.path.join(args.artifactpath, "files.json"))

# process the rest of binaries into ARTIFACTPATH
for root, _, files in os.walk(args.binarypath):
    for file in files:
        if file.endswith(".bin"):
            FILENAME = os.path.splitext(file)[0]
            FILEEXT = os.path.splitext(file)[1][1:]

            shutil.copyfile(os.path.join(root, file), os.path.join(args.artifactpath, f'{FILENAME}.{FileExtension}.{FILEEXT}'))
            
################## handle Github_Outputs ####################
print(f"version={VERSION}")
print(f"build={int(args.build)}")
print(f"stage={args.stage}")
