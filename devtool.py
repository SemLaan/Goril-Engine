import os
import sys
import subprocess

if __name__ == "__main__":
    # Call premake
    subprocess.run(("./vendor/bin/premake/premake5.exe", "vs2022"))