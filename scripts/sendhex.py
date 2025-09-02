# Copyright (c) 2025 Daniel Cliche
# SPDX-License-Identifier: MIT

import sys
import serial
import time

def send(ser, bytes):
    ser.write(bytes)

def main(argv):
    if (len(argv) < 2):
        print("Usage: sendhex.py <serial device> <hex file>")
        exit(0)
    else:
        try:
            ser = serial.Serial(argv[0], baudrate=115200)
        except serial.serialutil.SerialException:
            print("Unable to open the serial device {0}".format(argv[0]))
            exit(1)
        file = open(argv[1], 'r')
        lines = file.read().splitlines()
        lines = list(filter(None, lines))   # remove empty lines

        length = len(lines)
        send(ser, length.to_bytes(4, 'big'))

        cnt = 0
        for line in lines:
            if (line == '0'):
                line = '00000000'
            send(ser, bytearray.fromhex(line))
            cnt = cnt + 4
        time.sleep(2)
        ser.close()
    exit(0)


if __name__ == "__main__":
    main(sys.argv[1:])
