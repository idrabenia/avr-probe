#define __AVR_ATmega328__

#define F_CPU 16000000
#define BAUD 9600L
#define UBRRL_value (F_CPU / (BAUD * 16)) - 1
#define BUF_SIZE 30

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

void send_UART(char value);

void send_UART_str(char* value);

int main (void) 
{
    // Configure CS, MOSI and CLK
    DDRB |= _BV(DDB2) | _BV(DDB3) | _BV(DDB5);
    SPCR |= _BV(SPE) | _BV(MSTR) | _BV(CPOL) | _BV(CPHA);
    SPCR |= _BV(SPR1); // Clk / 64
    //SPSR |= _BV(SPI2X); // Double SPI Speed

    // Configure UART
    UBRR0L = UBRRL_value;
    UBRR0H = UBRRL_value >> 8;
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);

    // Disable CS
    PORTB |= _BV(PORTB2);
    
    char buffer[BUF_SIZE];

    while (1) {
        PORTB &= ~_BV(PORTB2);

        SPDR = _BV(3) | _BV(4);
        while (!(SPSR & _BV(SPIF)));

        SPDR = 0x00;
        while (!(SPSR & _BV(SPIF)));

        char* signal_str = itoa(SPDR, buffer, 10);
        send_UART_str(strcat(signal_str, "\n"));
        PORTB |= _BV(PORTB2);

        _delay_ms(100);
    }
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