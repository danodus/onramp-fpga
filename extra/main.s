=__start
	; format indicator
	"~Onr~amp~   "
	; jump to start
	jmp ^start

; Horizontal line
=hline
	enter
	push r0
	push r1
	push r2
	push r3
	imw r0 0x25000000
	ims r1 0x1010
	imw r2 0
:loop
	sts r1 r0 r2
	add r2 r2 2
	ltu r3 r2 20
	jnz r3 &loop
	pop r3
	pop r2
	pop r1
	pop r0
	leave
	ret


:start
	; call initialize
	call ^initialize

	call ^hline

	; tail-call
	mov r0 42	; exit code (42)
	jmp ^exit
