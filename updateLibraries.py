import os
import shutil

base_path = os.path.realpath(os.getcwd())
custom_lib_path = base_path + "/custom_firmware/"
original_lib_path = base_path + "/.pio/libdeps/adafruit_feather_esp32/Emotibit FeatherWing/"

custom_lib_files = [f for f in os.listdir(custom_lib_path) if os.path.isfile(os.path.join(custom_lib_path, f)) and (f.endswith(".h") or f.endswith(".cpp"))]
for f in custom_lib_files:
    shutil.copyfile(custom_lib_path + f, original_lib_path + f)
