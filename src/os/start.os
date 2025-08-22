; Copyright (c) 2025 Daniel Cliche
; SPDX-License-Identifier: MIT

=__start
    "~Onr~amp~   "      ; The executable file identifier; see "File Format" in the VM spec
    call ^main
:loop
    jmp &loop
