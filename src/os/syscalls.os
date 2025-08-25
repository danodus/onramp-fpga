; Copyright (c) 2025 Daniel Cliche
; SPDX-License-Identifier: MIT

; ==========================================================
; void __syscall(...);
; ==========================================================
; The syscall handler.
;
; The system call number is passed in r9. Arguments to the syscall are passed
; in r0-r3 as normal.
; ==========================================================

@__syscall

    ; get the syscall into r7
    imw r8 ^__process_info_table
    ldw r8 rpp r8     ; r8 = process_info_table
    ldw r8 r8 8       ; r8 = syscall table
    shl r7 r9 3       ; r7 = byte offset of syscall (r9 << 3)
    add r7 r8 r7      ; r7 = address of syscall

    ; tail-call it
    ldw r9 r7 4       ; r9 = syscall context
    ldw rip r7 0      ; rip = syscall rip


; ==========================================================
; int __sys_fwrite(int handle, const void* buffer, unsigned size);
; ==========================================================

=__sys_fwrite
    mov r9 6
    jmp ^__syscall

