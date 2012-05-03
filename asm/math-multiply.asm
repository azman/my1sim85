; showing 8-bit multiplication
; - done in sub-routine
; - input: multiplicand in b and multiplier in c
; - result: 16-bit in register pair bc

org 0000h
lxi sp,4000h
mvi b,10h
mvi c,10h
call do_mult
hlt

do_mult:
push h
push d
xra a
mov d,a
mov e,a
mov h,a
mov l,b
mov a,c
mvi b,8 ; loop 8-times (8-bit)
loop:
rrc
jnc skip
xchg
dad d
xchg
skip:
dad h
dcr b
jnz loop
mov b,d
mov c,e
pop d
pop h
ret
