# Change to C++17
# Automatically performs common transforms 
# in the *.h and *.cpp files in this directory
# to conform to the C++ 17 Standard
# By Juniper Foote

import re
from pathlib import Path


if __name__ == "__main__":
    headers = list(Path(".").resolve().rglob("*.h"))
    cpp_files = list(Path(".").resolve().rglob("*.cpp"))
    all_files = headers
    all_files.extend(cpp_files)
    for t_file in all_files:
        if t_file.name == "Words.h":
            continue
        in_text = t_file.read_text()
        out_text = re.sub(r"\) ?(const)?[\s\n]*throw\(\)[\s\n]*([\{=;:])", r") \g<1> noexcept \g<2>", in_text)
        out_text = re.sub(r"\) ?(const)?[\s\n]*throw\([\:A-Za-z0-9,\n\t _]+\)[\s\n]*([\{=;:])", r") \g<1> \g<2>", out_text)
        out_text = re.sub(r"(\W)ubyte(\W)", r"\g<1>uint8_t\g<2>", out_text)
        out_text = re.sub(r"(\W)byte(\W)", r"\g<1>int8_t\g<2>", out_text)
        t_file.write_text(out_text)
