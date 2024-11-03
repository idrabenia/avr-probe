#define __AVR_ATmega328__

#define F_CPU 8000000
#define BAUD 38400L
#define UBRRL_value (F_CPU / (BAUD * 16)) - 1
#define BUF_SIZE 30

#undef DEBUG

// State
#define INIT 0
#define ONE 1
#define ONE_MIDDLE 2
#define ZERO 3
#define ZERO_MIDDLE 4

// Input Signal
#define EMPTY 0
#define ZERO_SHORT 1
#define ZERO_LONG 2
#define ONE_SHORT 3
#define ONE_LONG 4

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

// variables to convert RC5 pulses to binary value on-fly
volatile int calls_count = 0;
volatile short prev_signal = 0;
volatile int tmp_result = 0;
volatile int cur_result = -1;
volatile short cur_state = INIT;

// calculate time in ms from MCU start 
volatile long app_time_ms = 0;
volatile short app_time_pulse = 0;

// time when last command was received
volatile long last_cmd_time = 0;

void send_uart(char value);

void send_uart_str(char* value);

void send_uart_hex(int hex);

void on_next_pulse(short signal, short* state, int* result);

short to_pulse_type(short singal, short duration);

int main (void) 
{
    // Configure UART
    UBRR0L = UBRRL_value;
    UBRR0H = UBRRL_value >> 8;
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);

    // PWM
    TCCR1A |= (1 << COM1A0);
    TCCR1B |= /*(1 << CS10)|(1 << CS11)|*/(1 << CS11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);
    TIMSK1 |= (1 << ICIE1);
    ICR1 = F_CPU / 8 / 10000;

    // Enable Port for LED
    DDRB |= (1 << PORTB2);
    PORTB &= ~(1 << PORTB2);

    sei();

    while (1) {
        int code = cur_result;

        if (code != -1) {
            if (app_time_ms - last_cmd_time > 300) {
                send_uart_str("Addr: ");
                send_uart_hex((code >> 6) & 0x1F);
                send_uart_str(", Command: ");
                send_uart_hex(code & 0x3F);
                send_uart('\n');
                last_cmd_time = app_time_ms;

                if ((code & 0x3F) == 1) {
                    PORTB |= (1 << PORTB2);
                } else if ((code & 0x3F) == 0) {
                    PORTB &= ~(1 << PORTB2);
                }
            }

            cur_result = -1;
        }

        _delay_ms(50);
    }
}

ISR(TIMER1_CAPT_vect) {
    short sig_val = PINB & (1 << PINB0);

    if (prev_signal == sig_val) {
        calls_count = (calls_count < 50) ? calls_count + 1 : 50;

        if (calls_count > 40 && cur_state != INIT) {
            cur_state = INIT;
            cur_result = tmp_result;
            tmp_result = 0;
        }
    } else {
        short cur_pulse_type = to_pulse_type(prev_signal, calls_count);
        on_next_pulse(cur_pulse_type, &cur_state, &tmp_result);
        calls_count = 0;
        prev_signal = sig_val;
    }

    if (app_time_pulse == 10) {
        app_time_ms += 1;
        app_time_pulse = 0;
    } else {
        app_time_pulse += 1;
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

short to_pulse_type(short signal, short duration)
{
    if (signal) {
        return (duration >= 5 && duration <= 12) ? ONE_SHORT : ONE_LONG;
    } else {
        return (duration >= 5 && duration <= 12) ? ZERO_SHORT : ZERO_LONG;
    }
}

void on_next_pulse(short signal, short* state, int* result)
{
    if (signal == EMPTY) {
        return;
    }

    switch (*state) {
        case INIT: {
            if (signal == ONE_SHORT) {
                *state = ONE;
                *result = (*result << 1) | 1;
            }

            break;
        }

        case ONE: {
            if (signal == ZERO_SHORT) {
                *state = ONE_MIDDLE;
                *result = (*result << 1) | 1;
            }

            break;
        }

        case ONE_MIDDLE: {
            if (signal == ONE_SHORT) {
                *state = ONE;
            } else if (signal == ONE_LONG) {
                *state = ZERO_MIDDLE;   
                *result <<= 1;
            }

            break;
        }

        case ZERO_MIDDLE: {
            if (signal == ZERO_SHORT) {
                *state = ZERO;
            } else if (signal == ZERO_LONG) {
                *state = ONE_MIDDLE;
                *result = (*result << 1) | 1;
            }

            break;
        }

        case ZERO: {
            if (signal == ONE_SHORT) {
                *state = ZERO_MIDDLE;
                *result <<= 1;
            }

            break;
        }

        default:
            break;
    }
}
