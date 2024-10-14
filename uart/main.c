#define __AVR_ATmega328__
#define F_CPU 16000000
#define BAUD 9600L
#define UBRRL_value (F_CPU / (BAUD * 16)) - 1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "string.h"

volatile uint8_t buff[30];
volatile uint8_t size;

void init_USART()
{
    UBRR0L = UBRRL_value;
    UBRR0H = UBRRL_value >> 8;
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
    sei();
}

void send_UART(char value) 
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = value;
}

void send_UART_str(char* value) 
{
    int length = strlen(value);
    
    for (int i = 0; i < length; i += 1) {
        send_UART(value[i]); 
    }
}

ISR(USART_RX_vect)
{
    buff[size++] = UDR0;
}

int main(void)
{
    init_USART();
    send_UART_str("Initialized\r\n");

    while(1) {
        _delay_ms(1000);

        if (size == 0) {
            continue;
        }

        cli();
        send_UART_str("-> ");

        for (int i = 0; i < size; i += 1) {
            send_UART(buff[i]);
        }
        
        size = 0;
        sei();
    }
}
