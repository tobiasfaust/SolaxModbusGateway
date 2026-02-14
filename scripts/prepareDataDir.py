Import("env");
"""
PlatformIO build script that prepares the data directory by copying board-specific files.
This script is executed during the PlatformIO build process and performs the following:
- Checks if a board-specific directory exists in 'esp_files/{BOARD_NAME}/'
- If it exists: Copies all files from the board-specific directory to the project root,
    overwriting existing files (dirs_exist_ok=True allows overwriting)
- If it doesn't exist: Prints an error message
Args:
        env: PlatformIO environment object containing build configuration (e.g., env["BOARD"])
copytree() function:
        - Recursively copies an entire directory tree from source to destination
        - In this case: copies from "esp_files/{BOARD}/" to project root (".")
dirs_exist_ok parameter (dirs_exist_ok=True):
        - Allows the destination directory to already exist without raising an error
        - Enables overwriting of existing files in the destination
        - Without this parameter, copytree() would fail if the target directory already exists
"""
import sys, os, re;
from shutil import copytree;

# Print environment variables for debugging
#print("Environment dump:")
#for key, value in env.items():
#   print(f"  {key}: {value}")
#print()


data_master_dir = "esp_files";
if (os.path.exists(data_master_dir +"/"+ env["BOARD"])):
    copytree(data_master_dir +"/"+ env["BOARD"] + "/" , ".", dirs_exist_ok=True);
    print("copy board specific files from:<" + data_master_dir +"/"+ env["BOARD"] + "> to <root>");
else:
    print("path not exists: " + data_master_dir +"/"+ env["BOARD"] + "/");