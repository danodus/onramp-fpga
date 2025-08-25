; Copyright (c) 2025 Daniel Cliche
; SPDX-License-Identifier: MIT

=__start
    imw rsp 0x10040000  ; Stack pointer to the end of RAM (256 KiB)
    call ^main          ; BIOS initialization
    imw r0 ^process_info_table
    imw r1 0x10000000   ; Jump to RAM
    add rpp r1 0
    add rip r1 0
:loop
    jmp &loop

=process_info_table
    3                   ; version (3)
    0x10030000          ; heap start address (0x10030000)
    ^sys_call_table     ; Table of system calls
    -1                  ; File handle of input stream
    -1                  ; File handle of output stream
    -1                  ; File handle of error stream
    0                   ; Command-line arguments
    0                   ; Environment variables
    0                   ; Working directory
    0x0                 ; Capabilities
    25                  ; Number of system calls (25)
    12                  ; Number of entries in the process info table (12)

; tail-call
=sys_fwrite1
    ; save rpp
    sub rsp rsp 4
    stw rpp 0 rsp
    ; set rpp to 0
    mov rpp 0
    ; call sys_fwrite
    imw ra ^sys_fwrite
    sub rsp rsp 4       ; push return address
    add rb rip 8
    stw rb 0 rsp
    add rip rpp ra      ; call
    add rsp rsp 4       ; pop return address
    ; restore rpp
    ldw rpp 0 rsp
    add rsp rsp 4
    ; return
    ldw rip 0 rsp

; tail call
=sys_fwrite3
    ; return
    mov r0 42
    ;add rsp rfp 0
    ;ldw rfp 0 rsp
    ;add rsp rsp 4
    ldw rip 0 rsp

=sys_call_table
    0x00000000     ; 0
    0x00000000
    0x00000000     ; 1
    0x00000000
    0x00000000     ; 2
    0x00000000
    0x00000000     ; 3
    0x00000000
    0x00000000     ; 4
    0x00000000
    0x00000000     ; 5
    0x00000000
    ^sys_fwrite1   ; 6 - fwrite
    0x00000000
    0x00000000     ; 7
    0x00000000
    0x00000000     ; 8
    0x00000000
    0x00000000     ; 9
    0x00000000
    0x00000000     ; 10
    0x00000000
    0x00000000     ; 11
    0x00000000
    0x00000000     ; 12
    0x00000000
    0x00000000     ; 13
    0x00000000
    0x00000000     ; 14
    0x00000000
    0x00000000     ; 15
    0x00000000
    0x00000000     ; 16
    0x00000000
    0x00000000     ; 17
    0x00000000
    0x00000000     ; 18
    0x00000000
    0x00000000     ; 19
    0x00000000
    0x00000000     ; 20
    0x00000000
    0x00000000     ; 21
    0x00000000
    0x00000000     ; 22
    0x00000000
    0x00000000     ; 23
    0x00000000
    0x00000000     ; 24
    0x00000000


