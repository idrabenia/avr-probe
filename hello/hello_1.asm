
.include "./m328Pdef.inc"

start:
  sbi   DDRB, 4  ; Set PB5 as output

blink: 
  call delay_1
  sbi PORTB, 4

  call delay_1
  cbi PORTB, 4

  rjmp blink

delay_1:
  ldi r28, 0x80
  ldi r29, 0x3E

loop_0:
  ldi r17, 0xFF

loop_1:
  dec r17
  cpi r17, 0x00
  brne loop_1

  sbiw r28, 1
  brne loop_0

  ret
