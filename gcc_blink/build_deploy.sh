avr-gcc -Os -DF_CPU=8000000UL -mmcu=atmega328p -c -o main.o main.c
avr-gcc -mmcu=atmega328p main.o -o main
avr-objcopy -O ihex -R .eeprom main main.hex
avrdude -pm328 -cusbasp -b9600 -Uflash:w:./main.hex:i -B 10