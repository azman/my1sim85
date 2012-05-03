	cpu "8085.tbl"

	org 2000h
	lxi sp, 4000h
	mvi a, 82h
	out 83h
	xra a
	out 82h

	; main routine
loop:	call g_key
	mov b, a
	call p_seg
	mov a, b
	cpi 0Fh
	jnz loop
	rst 1

	; subroutine to wait and read encoder input
	; - data d3-d0 => pb3-pb0
	; - data available da => pb4
	; - actual value in a
g_key:	push h
	push b
k_dn:	in 81h
	ani 10h ; check da bit - keydown
	jz k_dn
k_up:	in 81h
	ani 10h ; check da bit - keyup
	jnz k_up
	in 81h
	ani 0fh
	; get actual value
	lxi h, t_key
	mvi b, 0
	mov c, a
	dad b
	mov a, m
	pop b
	pop h
	ret

	; table for 3x4 keypad (4x4 encoder)
t_key:	dfb 01h, 02h, 03h, 0ffh, 04h
	dfb 05h, 06h, 0ffh, 07h, 08h
	dfb 09h, 0ffh, 0Eh, 00h, 0Fh, 0ffh

	; subroutine to send data to 7-segment (data in acc)
	; - 7-segment is common-cathode (i/o board) => (pa7-pa0)
	; - sseg pin on i/o board need to be pulsed => (pc0)
	; - c0 is the common pin (pulled LO to enable) => (pc1)
p_seg:	push h
	push b
	lxi h, t_seg
	mvi b, 0
	mov c, a
	dad b
	mov a, m
	out 80h
	; just in case - set pc1 as LO
	mvi a, 02h
	out 83h
	;latch seven segment data
	mvi a, 00h
	out 83h
	mvi a, 01h
	out 83h
	pop b
	pop h
	ret

	; table for 7-seg
t_seg:	dfb 0fch, 60h, 0dah, 0f2h, 066h
	dfb 0b6h, 0beh, 0e0h, 0feh, 0e6h
	dfb 01h, 01h, 01h, 01h, 01h, 092h

	end