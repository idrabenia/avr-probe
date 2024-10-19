#define __AVR_ATtiny13__
#define F_CPU 9600000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

volatile uint8_t out = 0;

ISR(ADC_vect)
{
    uint8_t cur_adc = ADCH;
    OCR0A = cur_adc;

    out = ~out;
    if (out == 0) {
        PORTB &= ~(1 << PORTB1);
    } else {
        PORTB |= (1 << PORTB1);
    }
}

int main(void)
{
    // initialization

    // ADC
    DDRB &= ~(1 << DDB4);
    ADMUX |= (1 << MUX1) | (1 << ADLAR);
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) /*| (1 << ADPS2)*/;
    ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADATE);
    ADCSRA |= (1 << ADTS2);
    ADCSRA |= (1 << ADSC);

    // PWM
    TCCR0A |= (1 << COM0A1) | (1 << WGM00) | (1 << WGM01);
    TCCR0B |= (1 << CS00);
    DDRB |= (1 << DDB0) | (1 << DDB1);
    OCR0A = 127;

    // WatchDog
    wdt_reset();
    WDTCR |= (1 << WDE) | (1 << WDP1) | (1 << WDP2);

    sei();

    while (1) {
        _delay_ms(50);
        wdt_reset();
    }
}
