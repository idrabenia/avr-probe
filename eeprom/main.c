#define __AVR_ATtiny13__
#define F_CPU 9600000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t prev_state = 0;
volatile uint8_t state = 0;

void set_pwm_duty(uint8_t state);

ISR(INT0_vect)
{
    state = state >= 2 ? 0 : state + 1;
    set_pwm_duty(state);
}

int main(void)
{
    // initialization

    // PWM
    TCCR0A |= (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);
    TCCR0B |= (1 << CS00);
    DDRB |= (1 << DDB0);
    OCR0A = 127;

    // Button, INT0
    GIMSK |= _BV(INT0);
    PORTB |= _BV(PORTB1);

    // Restore Previous State
    EEARL = 0;
    EECR |= _BV(EERE);
    state = EEDR;
    set_pwm_duty(state);

    sei();

    while (1) {
        _delay_ms(50);

        if (prev_state == state) {
            continue;
        }

        prev_state = state;
        while(EECR & _BV(EEPE));

        EEARL = 0;
        EEDR = state;
        EECR |= _BV(EEMPE);
        EECR |= _BV(EEPE);
    }
}

void set_pwm_duty(uint8_t state) 
{
    switch (state) {
        case 0:
            OCR0A = 60;
            break;
        case 1:
            OCR0A = 127;
            break;
        case 2:
            OCR0A = 200;
            break;
        default:
            break;
    }
}