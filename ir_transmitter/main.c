#define __AVR_ATmega328__

// UART
#define F_CPU 16000000
#define BAUD 38400L
#define UBRRL_value (F_CPU / (BAUD * 16)) - 1
#define BUF_SIZE 30

// RC5
#define RC5_PULSE_ONE_US 950
#define RC5_PULSE_ZERO_US 800
#define PWM_FREQ 40000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

volatile short buff[30];
volatile short size = 0;
volatile short rc5_control_bit = 0;

void send_uart(char value);

void send_uart_str(char* value);

void send_uart_hex(int hex);

void send_rc5_cmd(short cmd, short* control_bit);

void send_rc5_one();

void send_rc5_zero();

void send_rc5_bit(short curBit);

int main (void) 
{
    // Configure UART
    UBRR0L = UBRRL_value;
    UBRR0H = UBRRL_value >> 8;
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);

    // PWM
    TCCR1A |= (1 << COM1A0);
    TCCR1B |= /*(1 << CS10)|(1 << CS11)|*/(1 << CS11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);
    // TIMSK1 |= (1 << ICIE1);
    ICR1 = F_CPU / 8 / PWM_FREQ / 2;
    DDRB &= ~(1 << DDB1);

    sei();

    while (1) {
        if (size > 0) {
            char cmd_str_buf[3] = { buff[0], buff[1], '\0' };
            short command = atoi(cmd_str_buf);
            size = 0;

            send_uart_hex(command);
            send_uart('\n');

            send_rc5_cmd(command, &rc5_control_bit);
            _delay_ms(100);
            send_rc5_cmd(command, &rc5_control_bit);

            rc5_control_bit = !rc5_control_bit;
        }

        // send_rc5_cmd(9);

        _delay_ms(200);
    }
}

ISR(USART_RX_vect)
{
    buff[size++] = UDR0;
}

void send_rc5_cmd(short cmd, short* control_bit)
{
    send_rc5_one();  // 1
    send_rc5_one();  // 1

    send_rc5_bit(*control_bit);

    send_rc5_zero(); // 0
    send_rc5_zero(); // 0
    send_rc5_zero(); // 0
    send_rc5_zero(); // 0
    send_rc5_zero(); // 0

    send_rc5_bit((cmd >> 5) & 0x01);
    send_rc5_bit((cmd >> 4) & 0x01);
    send_rc5_bit((cmd >> 3) & 0x01);
    send_rc5_bit((cmd >> 2) & 0x01);
    send_rc5_bit((cmd >> 1) & 0x01);
    send_rc5_bit((cmd >> 0) & 0x01);

    TCCR1A &= ~(1 << COM1A0);
    PORTB &= ~(1 << PORTB1);
    DDRB &= ~(1 << DDB1);
}

void send_rc5_one()
{
    // 1
    TCCR1A &= ~(1 << COM1A0);
    PORTB &= ~(1 << PORTB1);
    DDRB &= ~(1 << DDB1);
    _delay_us(RC5_PULSE_ZERO_US);

    TCCR1A |= (1 << COM1A0);
    DDRB |= (1 << DDB1);
    _delay_us(RC5_PULSE_ONE_US);
}

void send_rc5_zero() 
{
    // 0
    TCCR1A |= (1 << COM1A0);
    DDRB |= (1 << DDB1);
    _delay_us(RC5_PULSE_ONE_US);

    TCCR1A &= ~(1 << COM1A0);
    PORTB &= ~(1 << PORTB1);
    DDRB &= ~(1 << DDB1);
    _delay_us(RC5_PULSE_ZERO_US);
}

void send_rc5_bit(short curBit)
{
    if (curBit) {
        send_rc5_one();
    } else {
        send_rc5_zero();
    }
}

void send_uart(char value) 
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = value;
}

void send_uart_str(char* value) 
{
    int length = strlen(value);
    
    for (int i = 0; i < length; i += 1) {
        send_uart(value[i]); 
    }
}

void send_uart_hex(int value)
{
    char buf[30] = {0};
    char* hex_code = itoa(value, buf, 16);

    send_uart_str(hex_code);
}
