# This script converts an i18n file containing Japanese strings into Shift-JIS
# strings.

import os
import sys

file_contents = b""

with open(sys.argv[1], "rb") as f:
    lines = f.readlines()

new_lines = []

for line in lines:
    idx1 = line.find(b'"')
    idx2 = line.find(b'"', idx1 + 1)
    if idx1 == -1 or idx2 == -1:
        # skip this line because it contains no quotation marks
        new_lines.append(line)
        continue

    # check if this " is being escaped
    if line[idx2 - 1] == ord("\\"):
        # find the last " that's not escaped
        while idx2 != -1:
            if line[idx2 - 1] != ord("\\"):
                break

            idx2 = line.find(b'"', idx2 + 1)

    sjis_str = line[idx1 + 1 : idx2].decode("utf-8").encode("shift_jis")

    converted_str = b""

    i = 0
    while i < len(sjis_str):
        byte = sjis_str[i]

        if byte == ord("\\") and len(sjis_str) > (i + 1):
            second_char = sjis_str[i + 1]

            # handle escape sequences
            if second_char in [ord("r"), ord("n"), ord("\\"), ord('"'), ord("'")]:
                converted_str += b"\\" + bytes(chr(second_char), "utf-8")
                # skip the \ and escaped character
                i += 2
                continue

        converted_str += "\\x{:02X}".format(byte).encode("utf-8")
        i += 1

    new_line = line[: idx1 + 1] + converted_str + line[idx2:]
    new_lines.append(new_line)

file_contents = b"".join(new_lines)

if os.path.dirname(sys.argv[2]):
    os.makedirs(os.path.dirname(sys.argv[2]), exist_ok=True)

with open(sys.argv[2], "wb") as f:
    f.write(file_contents)
