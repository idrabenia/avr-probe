#define __AVR_ATtiny13__
#define F_CPU 9600000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t out = 0;

ISR(ADC_vect)
{
    /* Two Half-Period Rectifier */
    // uint8_t cur_adc = ADCH;
    // OCR0A = cur_adc > 64? cur_adc : 64 + (64 - cur_adc);

    /* Same Signal to Out */
    // OCR0A = ADCH;

    /* Digital Filter */
    uint8_t cur_adc = ADCH;
    out += (cur_adc - out) >> 1;
    OCR0A = out;

    /* Rectifier */
    // uint8_t cur_adc = ADCH;
    // OCR0A = cur_adc > 64? cur_adc : 64;

    /* Distortion */
    // uint8_t cur_adc = ADCH;
    // if (cur_adc > 70) {
    //     out = 70;
    // } else if (cur_adc < 0) {
    //     out = 0;
    // } else {
    //     out = cur_adc;
    // }
    // OCR0A = out;
}

int main(void)
{
    // initialization

    // ADC
    DDRB &= ~(1 << DDB4);
    ADMUX |= (1 << MUX1) | (1 << ADLAR);
    ADCSRA |= (1 << ADPS1) | (1 << ADPS2);
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
