/*7 segment display library - Sam Heil 13.01.16
Simply change the variable dispVal to whatever four digit
number you want to display. The 4 least significant bits of
decptVal control the decimal points in digits 1-4. This
program uses pins PD0-PD7 and PC0-PC3, as well as TIMER0.
Call initDisplay() before using. */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DELAY 16 /*Digit updates every DELAY*TIMER_DIV clock cycles*/
#define TIMER_DIV DIV_64
#define DIV_1 (1<<CS00)
#define DIV_8 (1<<CS01)
#define DIV_64 (1<<CS00) | (1<<CS01)
#define DIV_256 (1<<CS02)
#define DIV_1024 (1<<CS02) | (1<<CS00)

volatile uint16_t dispVal = 0xffff, _dispVar;
volatile uint8_t _dispPos = 0xff, decptVal = 0;

/*
Assumed connections:
7seg outputs on PORTD

   0
 |---|
3| 1 |5
 |---|
4|   |6
 |---|  .
   2    7

Common cathodes on PORTC

1: PC0
2: PC1
3: PC2
4: PC3
*/

inline void writeDisplay(uint8_t pos, uint8_t val) {
	PORTC |=  0x0f;
	PORTC &= ~(1<<pos);
	PORTD = val;
}

inline void initDisplay() {
	DDRD = 0xff;
	DDRC |= 0x0f;
	TCCR0A |= (1<<WGM01);
	TCCR0B |= TIMER_DIV;
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = DELAY;
	sei();
}

uint8_t digits[] = {0b01111101, 0b01100000, 0b00110111, 0b01100111,
   0b01101010, 0b01001111, 0b01011111, 0b01100001, 0b01111111, 0b01101111};
ISR(TIMER0_COMPA_vect) {
	if (dispVal < 10000) {
		if (_dispPos == 0xff) {
			_dispPos = 3;
			_dispVar = dispVal;
		}
		writeDisplay(_dispPos,digits[_dispVar%10]+((decptVal&(1<<_dispPos))<<(7-_dispPos)));
		_dispVar /= 10;
		_dispPos--;
	}
}
