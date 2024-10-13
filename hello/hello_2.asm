.list
.include "./m328Pdef.inc"

.dseg
.org 0x0100
    delay_val: .byte 1

.cseg
.org 0x0000
    rjmp button_int ; start

.org 0x0002 ; INT0 interruption
    rjmp button_int

.org INT_VECTORS_SIZE
start:
    cli

    ; Initialize Stack
    ldi r17, low(RAMEND)
    out SPL, r17
    ldi r17, high(RAMEND)
    out SPH, r17

    ; Store initial delay value
    ldi r17, 0x01
    sts delay_val, r17

    ; Initialize interruptions

    ; Trigger on rising edge
    ldi r17, (1 << ISC01) | (1 << ISC00) 
    sts EICRA, r17

    sbi EIMSK, INT0 ; Enable INT0 interruption with pull-up resistor
    sbi PORTD, PORTD2 ; Enable input mode for Pin 2 of the Port D 
    sbi DDRB, PORTB4 ; Set PB4 as output

    sei

main_loop:
    ldi r20, 70 ; delay_base that will be multiplied by delay_val
    lds r7, delay_val ; delay multiplier
    mul r7, r20 ; delay_base * delay_val

    rcall delay_1
    sbi PORTB, PORTB4 ; set "1"

    rcall delay_1
    cbi PORTB, PORTB4 ; set "0"

    ldi r21, 0 ; clear delay_val changed flag
    rjmp main_loop

delay_1:
    ldi r28, low(3000)
    ldi r29, high(3000)

delay_loop_0:
    mov r17, r0

delay_loop_1:
    dec r17
    cpi r17, 0x00
    brne delay_loop_1

    sbiw r28, 1
    brne delay_loop_0

    ret

button_int:
    in r3, SREG

    ldi r22, 1
    cpse r21, r22
    rjmp inc_delay
    rjmp exit_int

inc_delay:
    lds r19, delay_val
    inc r19
    ldi r18, 0x04
    cpse r18, r19
    rjmp save_delay
    ldi r19, 0x01

save_delay:
    sts delay_val, r19 ; save delay_val
    ldi r21, 1 ; set delay_val changed flag

exit_int:
    out SREG, r3
    reti
