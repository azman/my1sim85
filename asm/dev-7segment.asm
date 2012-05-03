; program for up/down 7-segment counter
; - with run/pause select and blinking decimal point
; - requires only ONE table
; connections:
; - PA0-PA7 (8255@80H) => (a,b,c,d,e,f,g,dot)
; - PB0 (8255@80H) => up/down select, 1=up, 0=down
; - PB1 (8255@80H) => run/pause select, 1=run, 0 = pause

org 0000h
lxi sp, 4000h
mvi a, 82h
out 83h

start_up: ; by default, count down!
lxi h, table
mvi b, 10
jmp loop

start_dn:
lxi h, table1
mvi b, 0
dcx h
inr b

loop:
call go_datab
call delay
call go_data
call delay

in 81h
ani 02h
jz loop
in 81h
ani 01h
jz go_dn
go_up:
mov a,b
ora a
jz start_up
inx h
dcr b
jnz loop
jmp start_up
go_dn:
mov a,b
cpi 10
jz start_dn
dcx h
inr b
mov a,b
cpi 11
jc loop
jmp start_dn

go_datab:
mov a, m
xri 80h
jmp latch
go_data:
mov a, m
latch:
out 80h
ret

delay:
push psw
push b
lxi b, 4
delay_loop:
dcx b
mov a, b
ora c
jnz delay_loop
pop b
pop psw
ret

table:
dfb 3fh, 06h, 5bh, 4fh, 66h,
dfb 6dh, 7dh, 07h, 7fh, 6fh
table1:
dfb 00h
