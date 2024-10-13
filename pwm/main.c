#define __AVR_ATmega328__

#include <avr/io.h>
#include <util/delay.h>

int main (void) 
{
    DDRB |= (1 << DDB1); // PB1 as output
    ICR1 = 0x000F;
    OCR1A = 0x0008; // set PWM for 50% duty cycle at 10bit
    TCCR1A |= (1 << COM1A1); // set non-inverting mode
    TCCR1A |= (1 << WGM11);  // set 10bit phase corrected PWM Mode
    TCCR1B |= (1 << CS10) | (1 << CS12) | (1 << WGM13) | (1 << WGM12); // set prescaler to 8 and starts PWM
    _delay_ms(1000);
    
    while (1)
    {
        OCR1A = 0x01FF;
        ICR1 = 0x03FF;
        _delay_ms(1000);

        OCR1A = 0x00FF;
        ICR1 = 0x03FF;
        _delay_ms(1000);

        while (TCNT1 > 0x01FF) { /* noop */ }
        OCR1A = 0x00FF;
        ICR1 = 0x01FF;
        _delay_ms(1000);

        OCR1A = 0x008F;
        ICR1 = 0x01FF;
        _delay_ms(1000);
    }
}
