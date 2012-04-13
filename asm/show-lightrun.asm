	org 0000h
	jmp start

	org 0024h
	jmp do_trap

	org 002ch
	pop psw ; take out 
	hlt

	org 003ch
	jmp do_i7p5

	org 0040h
start:	lxi sp, 4000h
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

	; basic delay routine
delay:	push b
	lxi b, 8
delay_loop:
	dcx b
	mov a, b
	ora c
	jnz delay_loop
	pop b
	ret

	; interrupt service routine (trap)
do_trap:
	push psw
	rim
	out 80h
	in 81h
	ani 80h ; check initial condition
	jnz trap1
trap0:	in 81h
	ani 80h ; wait for inverse state
	jz trap0
	jmp trapz
trap1:	in 81h
	ani 80h ; wait for inverse state
	jnz trap1
trapz:	mvi a,0x1bh ; unmask 7.5, reset ff
	sim
	pop psw
	ei
	ret

	; interrupt service routine (i7p5)
do_i7p5:
	push psw
	in 81h
	ani 40h ; check initial condition
	jnz i7p51
i7p50:	in 81h
	ani 40h ; wait for inverse state
	jz i7p50
	jmp i7p5z
i7p51:	in 81h
	ani 40h ; wait for inverse state
	jnz i7p51
i7p5z:	mvi a,0x1ch ; unmask 6.5 and 5.5, reset ff
	sim
	pop psw
	ei
	ret
