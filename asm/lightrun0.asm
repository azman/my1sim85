	org 2000h
	lxi sp, 4000h
	mvi a, 83h
	out 83h
	mvi c, 01h
	mov a, c
	out 80h

	; main routine
loop:	mov a, c
	rlc ; rrc for other direction
	mov c, a
	out 80h
skip:	call delay
	jmp loop

delay:	push b
	lxi b, 64000
delay_loop:	dcx b
	mov a, b
	ora c
	jnz delay_loop
	pop b
	ret
