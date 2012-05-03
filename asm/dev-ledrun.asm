	org 0000h
	lxi sp, 4000h
	mvi a, 83h
	out 83h
	mvi a, 01h ; init pattern

	; main routine
loop:	out 80h
	call delay
	rlc
	jmp loop

	; basic delay routine
delay:	push b
	mvi b, 8
delay_loop:
	dcr b
	jnz delay_loop
	pop b
	ret
