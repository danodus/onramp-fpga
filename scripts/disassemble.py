#!/usr/bin/env python

# Copyright (c) 2025 Fraser Heavy Software
# SPDX-License-Identifier: MIT

# Given an Onramp bytecode program that has corresponding debug info, this
# produces a bytecode and disassembly printout of the program.

import sys

OPCODES = [
    "add", "sub", "mul", "divu",
    "and", "or",  "shl", "shru",
    "ldw", "stw", "ldb", "stb",
    "ims", "ltu", "jz",
]

REGISTERS = [
    "r0",  "r1",  "r2",  "r3",
    "r4",  "r5",  "r6",  "r7",
    "r8",  "r9",  "ra",  "rb",
    "rsp", "rfp", "rpp", "rip",
]

def print_mix(x):
    print(" ", end="")
    if 0x80 <= x <= 0x8F:
        print(REGISTERS[x - 0x80], end="")
    elif x >= 0x90:
        print(x - 256, end="")
    else:
        print(x, end="")

with open(sys.argv[1] + ".od", "r") as debuginfo:
    lines = debuginfo.readlines()
with open(sys.argv[1], "rb") as program:
    bytecode = program.read()

total = 0
for line in lines:
    if line.startswith("#symbol "):
        print("\n; @" + hex(total), line.split()[1])
    elif line.startswith("#") or line.startswith(";"):
        pass
    else:
        count = int(line)

        for i in range(0, count, 4):
            start = total
            step = min(4, count - i)
            for j in range(step):
                print(" {:02x}".format(bytecode[total]), end="")
                total += 1
            if step == 4:
                opcode = bytecode[start]
                if 0x70 <= opcode <= 0x7E:
                    print("  ; " + OPCODES[opcode - 0x70], end="")
                    print_mix(bytecode[start + 1])
                    if opcode == 0x7C or opcode == 0x7E:
                        value = bytecode[start + 2] + bytecode[start + 3] * 256
                        if opcode == 0x7C: # ims
                            print("", value, end="")
                        elif opcode == 0x7E: # jz
                            if value >= 0x8000:
                                value -= 0x10000
                            print(" {:+d}".format(value), end="")
                    else:
                        print_mix(bytecode[start + 2])
                        print_mix(bytecode[start + 3])
            print()
