;hello.asm
;  turns on an LED which is connected to PB5 (digital out 13)

.include "./m328Pdef.inc"

	ldi r16,0b00100000
	out DDRB,r16
	out PortB,r16

Start:
	neg r16
	out PortB, r16

	ldi r18, 0xFF
Loop0:
	dec r18

	ldi r17, 0xFF
Loop1:
	dec r17
	cpi r17, 0x00
	brne Loop1

	cpi r18, 0x00
	brne Loop0
	
	rjmp Start

