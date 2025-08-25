; Copyright (c) 2025 Daniel Cliche
; SPDX-License-Identifier: MIT

=__start
    "~Onr~amp~   "      ; The executable file identifier; see "File Format" in the VM spec
    ; Check that VM version is 3
    ldw r1 r0 0
    sub r2 r1 3
    jnz r2 &__start_version_fail

    ; Version is good, jump into C
    mov r1 rsp
    call ^__start_c
:loop
    jmp &loop

:__start_version_fail
    imw r1 0x20000000
    imw r0 255
    stw r0 r1 0    
:loop2
    jmp &loop2

=__call_constructor
    add rip rpp r3

=__call_destructor
    add rip rpp r0
