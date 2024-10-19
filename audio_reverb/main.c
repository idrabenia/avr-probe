#define __AVR_ATtiny13__
#define F_CPU 1200000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t out = 0;
volatile uint8_t buffer[64] = {0};
volatile uint16_t counter = 0;

ISR(ADC_vect)
{
    uint8_t cur_adc = ADCH;

    out = (buffer[counter] >> 1) + cur_adc;
    OCR0A = out;

    buffer[counter] = out;
    counter = (counter >= 55) ? 0 : counter + 1;
}

int main(void)
{
    // initialization

    // ADC
    DDRB &= ~(1 << DDB4);
    ADMUX |= (1 << MUX1) | (1 << ADLAR);
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
    ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADATE);
    ADCSRA |= (1 << ADSC);

    // PWM
    TCCR0A |= (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);
    TCCR0B |= (1 << CS00);
    DDRB |= (1 << DDB0);
    OCR0A = 127;

    sei();

    while (1) {
        _delay_ms(50);
    }
}
