; showing 8-bit unsigned division
; - done in sub-routine
; - input: dividend in b and divisor in c
; - result: quotient in b and remainder in c

org 0000h
lxi sp,4000h
mvi b,20h
mvi c,30h
call do_divs
hlt

do_divs:
push h
mov l,b
mvi b,8 ; loop 8-times (8-bit)
xra a
mov h,a
loop:
dad h
mov a,h
cmp c
jc next
sub c
mov h,a
mov a,l
ori 01h
mov l,a
next:
dcr b
jnz loop
mov b,l
mov c,h
pop h
ret
