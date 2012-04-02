	org 0000h
	lxi sp, 4000h
	mvi a, 83h
	out 83h
	mvi c, 01h ; init pattern
	mov a, c

	; main routine
loop:	out 80h
	call delay
	in 81h
	ani 01h ; check run/stop switch
	jnz skip
	in 81h
	ani 02h ; check direction switch
	mov a, c
	jz runleft
	rrc
	jmp runsave
runleft:
	rlc
runsave:
	mov c, a
skip:	jmp loop

delay:	push b
	lxi b, 8
delay_loop:
	dcx b
	mov a, b
	ora c
	jnz delay_loop
	pop b
	ret
