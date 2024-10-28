#define __AVR_ATmega328__

#define F_CPU 16000000
#define BAUD 9600L
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

volatile int calls_count = 0;
volatile short signal = 0;
volatile short delays[100] = {EMPTY};
volatile short delays_cnt = 0;

#ifdef DEBUG
volatile short durations[100] = {0};
volatile short signals[100] = {0};
#endif

void send_UART(char value);

void send_UART_str(char* value);

void send_UART_hex(int hex);

void on_next_pulse(short signal, short* state, int* result);

short to_pulse_type(short singal, short duration);

int get_rc5_code(short* delays, short* delays_size);

#ifdef DEBUG

char* state_as_str(short state);

char* signal_as_str(short signal);

#endif

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

    sei();
    char buf[30] = {0};
    delays_cnt = 0;

    while (1) {
        if (delays_cnt > 0) {

            #ifdef DEBUG
            for (int i = 0; i < delays_cnt; i += 1) {
                send_UART_hex(signals[i]);
                send_UART(' ');
                send_UART_hex(durations[i]);
                send_UART('\n');
                signals[i] = -1;
                durations[i] = 0;
            }
            #endif

            int code = get_rc5_code((short*) &delays, (short*) &delays_cnt);
            delays_cnt = 0;

            send_UART_str("Addr: ");
            send_UART_hex((code >> 6) & 0x1F);
            send_UART_str(", Command: ");
            send_UART_hex(code & 0x3F);
            send_UART('\n');
        }

        _delay_ms(500);
    }
}

ISR(TIMER1_CAPT_vect) {
    short sig_val = PINB & (1 << PINB0);

    if (signal != sig_val) {
        #ifdef DEBUG
        durations[delays_cnt] = calls_count;
        signals[delays_cnt] = signal;
        #endif

        delays[delays_cnt] = to_pulse_type(signal, calls_count);
        delays_cnt = (delays_cnt <= 99) ? delays_cnt + 1 : 0;
        calls_count = 0;
        signal = sig_val;
    } else {
        // if (calls_count < 25) {
            calls_count += 1;
        // } else {
            // delays[delays_cnt] = EMPTY;
        // }
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

void send_UART_hex(int value)
{
    char buf[30] = {0};
    char* hex_code = itoa(value, buf, 16);

    send_UART_str(hex_code);
}

short to_pulse_type(short singal, short duration) 
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
        #ifdef DEBUG
        send_UART_str("State=");
        send_UART_str(state_as_str(*state));
        send_UART_str(", Signal=");
        send_UART_str(signal_as_str(signal));
        send_UART_str(", Result=");
        send_UART_hex(*result);
        send_UART('\n');
        #endif

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
            }

            break;
        }

        case ONE_MIDDLE: {
            if (signal == ONE_SHORT) {
                *result = (*result << 1) | 1;
                *state = ONE;
            } else if (signal == ONE_LONG) {
                *result = (*result << 1) | 1;
                *state = ZERO_MIDDLE;   
            }

            break;
        }

        case ZERO_MIDDLE: {
            if (signal == ZERO_SHORT) {
                *state = ZERO;
                *result <<= 1;
            } else if (signal == ZERO_LONG) {
                *state = ONE_MIDDLE;
                *result <<= 1;
            }

            break;
        }

        case ZERO: {
            if (signal == ONE_SHORT) {
                *state = ZERO_MIDDLE;
            }

            break;
        }

        default:
            break;
    }

    #ifdef DEBUG
    send_UART_str("State=");
    send_UART_str(state_as_str(*state));
    send_UART_str(", Signal=");
    send_UART_str(signal_as_str(signal));
    send_UART_str(", Result=");
    send_UART_hex(*result);
    send_UART('\n');
    #endif
}

int get_rc5_code(short* delays, short* delays_size)
{
    short state = INIT;
    int result = 0;

    for (short i = 0; i < *delays_size; i += 1) {
        on_next_pulse(delays[i], &state, &result);
        delays[i] = EMPTY;
    }

    if (state == ZERO_MIDDLE) {
        result <<= 1;
    }

    *delays_size = 0;
    return result;
}

#ifdef DEBUG

char* state_as_str(short state) {
    switch (state) {
        case INIT:
            return "INIT";
        case ONE:
            return "ONE";
        case ONE_MIDDLE:
            return "ONE_MIDDLE";
        case ZERO:
            return "ZERO";
        case ZERO_MIDDLE:
            return "ZERO_MIDDLE";
    }
}

char* signal_as_str(short signal) {
    switch (signal) {
        case ONE_SHORT:
            return "ONE_SHORT";
        case ONE_LONG:
            return "ONE_LONG";
        case ZERO_SHORT:
            return "ZERO_SHORT";
        case ZERO_LONG:
            return "ZERO_LONG";
    }
}

#endif
