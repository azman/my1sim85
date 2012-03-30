	org 0000h
	lxi sp, 4000h
	mvi a, 83h
	out 83h
	mvi c, 01h
	mov a, c
	out 80h

	; main routine
loop:	in 81h
	ani 01h ; check run/stop switch
	jnz skip
	in 81h
	ani 02h ; check direction switch
	jz runleft
	mov a, c
	rrc
	jmp runsave
runleft:
	mov a, c
	rlc
runsave:
	mov c, a
	out 80h
skip:	call delay
	jmp loop

delay:	push b
	lxi b, 8
delay_loop:
	dcx b
	mov a, b
	ora c
	jnz delay_loop
	pop b
	ret
