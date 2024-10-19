avr-gcc -Os -DF_CPU=1200000UL -mmcu=attiny13 -c -o main.o main.c
avr-gcc -mmcu=attiny13 main.o -o main
avr-objcopy -O ihex -R .eeprom main main.hex
avrdude -pt13 -cusbasp -b9600 -Uflash:w:./main.hex:i -B 10
# avrdude -pm328p -carduino -b57600 -P /dev/cu.usbserial-1410 -Uflash:w:./main.hex:i
