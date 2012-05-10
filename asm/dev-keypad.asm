org 0000h
lxi sp, 4000h
mvi a, 82h
out 83h

; main routine
loop:
call g_key
mov b, a
call p_seg
mov a, b
cpi 0Fh
jnz loop
hlt

KEYCHK:	equ 81h

; subroutine to wait and read encoder input
; - data d3-d0 => pb3-pb0
; - data available da => pb4
; - actual value in a
g_key:
push h
push b
k_dn:
in KEYCHK
ani 10h ; check da bit - keydown
jz k_dn
k_up:
in KEYCHK
ani 10h ; check da bit - keyup
jnz k_up
in KEYCHK
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
t_key:
dfb 01h, 02h, 03h, 0ffh, 04h
dfb 05h, 06h, 0ffh, 07h, 08h
dfb 09h, 0ffh, 0Eh, 00h, 0Fh, 0ffh

SEGCHK:	equ 80h

; subroutine to send data to 7-segment (common-cathode)
; - data index in acc 
; - dot => bit7
; - g,f,...,a => bit6-bit0
p_seg:
push h
push b
lxi h, t_seg
mvi b, 0
mov c, a
dad b
mov a, m
out SEGCHK
pop b
pop h
ret

; table for 7-seg
t_seg:
dfb 3fh, 06h, 5bh, 4fh, 66h,
dfb 6dh, 7dh, 07h, 7fh, 6fh
dfb 80h, 80h, 80h, 80h, 80h, 49h

end
