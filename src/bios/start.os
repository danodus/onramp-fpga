=__start
    "~Onr~amp~   "    ; The executable file identifier; see "File Format" in the VM spec
    imw rsp 0x10003000
    mov r1 rsp
    call ^main
:loop
    jmp &loop
