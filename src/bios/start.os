; Copyright (c) 2025 Daniel Cliche
; SPDX-License-Identifier: MIT

=__start
    imw rsp 0x11fc0000  ; Stack pointer to the end of RAM (32 MiB) - 256 KiB for BIOS globals 
    call ^main          ; BIOS initialization
    imw r0 ^process_info_table
    imw r1 0x10000000   ; Jump to RAM
    add rpp r1 0
    add rip r1 0
:loop
    jmp &loop

=args
    0x00000000

=env
    0x00000000

=process_info_table
    3                   ; version
    0x10030000          ; heap start address
    ^sys_call_table     ; Table of system calls
    0                   ; File handle of input stream
    1                   ; File handle of output stream
    2                   ; File handle of error stream
    ^args               ; Command-line arguments
    ^env                ; Environment variables
    0                   ; Working directory
    0x0                 ; Capabilities
    25                  ; Number of system calls
    12                  ; Number of entries in the process info table

=sys_exit1
    mov rpp 0           ; set rpp to BIOS
    call ^sys_exit
    ; Does not return

=sys_time1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_time
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_fopen1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_fopen
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_fclose1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_fclose
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_fread1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_fread
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_fwrite1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_fwrite
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_fseek1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_fseek
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_ftell1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_ftell
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_ftrunc1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_ftrunc
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_dopen1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_dopen
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_dread1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_dread
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_stat1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_stat
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret    

=sys_rename1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_rename
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret    

=sys_unlink1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    call ^sys_unlink
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_missing1
    sub rsp rsp 4       ; save rpp
    stw rpp 0 rsp
    mov rpp 0           ; set rpp to BIOS
    mov r0 r9           ; first arg is the call number
    call ^sys_missing
    ldw rpp 0 rsp       ; restore rpp
    add rsp rsp 4
    ret

=sys_call_table
    ^sys_exit1     ; 0 - exit
    0x00000000
    ^sys_missing1  ; 1
    0x00000001
    ^sys_time1     ; 2 - time
    0x00000002
    ^sys_fopen1    ; 3 - fopen
    0x00000003
    ^sys_fclose1   ; 4 - fclose
    0x00000004
    ^sys_fread1    ; 5 - fread
    0x00000005
    ^sys_fwrite1   ; 6 - fwrite
    0x00000006
    ^sys_fseek1    ; 7 - fseek
    0x00000007
    ^sys_ftell1    ; 8 - ftell
    0x00000008
    ^sys_ftrunc1   ; 9 - ftrunc
    0x00000009
    ^sys_dopen1    ; 10 - dopen
    0x0000000A
    ^sys_missing1  ; 11
    0x0000000B
    ^sys_dread1    ; 12 - dread
    0x0000000C
    ^sys_stat1     ; 13 - stat
    0x0000000D
    ^sys_rename1   ; 14 - rename
    0x0000000E
    ^sys_missing1  ; 15
    0x0000000F
    ^sys_unlink1   ; 16 - unlink
    0x00000010
    ^sys_missing1  ; 17
    0x00000011
    ^sys_missing1  ; 18
    0x00000012
    ^sys_missing1  ; 19
    0x00000013
    ^sys_missing1  ; 20
    0x00000014
    ^sys_missing1  ; 21
    0x00000015
    0x00000000     ; 22 - debug (not available)
    0x00000016
    ^sys_missing1  ; 23
    0x00000017
    ^sys_missing1  ; 24
    0x00000018


