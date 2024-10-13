avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o main.o main.c
avr-gcc -mmcu=atmega328p main.o -o main
avr-objcopy -O ihex -R .eeprom main main.hex
# avrdude -pm328 -cusbasp -b57600 -Uflash:w:./main.hex:i -B 10
avrdude -pm328p -carduino -b57600 -P /dev/cu.usbserial-1410 -Uflash:w:./main.hex:i
