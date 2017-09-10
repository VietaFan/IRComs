/*
 * radar.c - Uses an AVR ATMega328, a photodiode, and an infrared LED
 * to detect nearby objects. Displays the result using a 7 segment display.
 */

#include <avr/io.h>
#include <util/delay.h>
#include "7seglib.c"

/*
 * This assumes connections to 7 segment display described in 7seglib.c.
 * Indicator LED on pin PB1 will be on if there is an object detected.
 */
inline uint16_t getADCVal(uint8_t diodeNum) {
    ADMUX &= 254;
    ADMUX |= diodeNum;
    ADCSRA |=  (1<<ADSC);
    while (ADCSRA & (1<<ADSC));
    return ADC;
}

int main() {
	// set up input and output pins
    DDRD = 0xff;
    DDRC = 0x0f;
    DDRB |= (1<<0);
    initDisplay();
    // set up ADC for photodiode reading
    ADMUX = PC4; 
    ADCSRA |= (1<<ADPS1) | (1<<ADPS0);
    ADMUX |= (1<<REFS0);
    ADCSRA |= (1<<ADEN);
    int16_t cur0, prev0;
    cur0 = getADCVal(0);
    int16_t olds0[16];
    for (uint8_t i=0; i<16; i++)
		olds0[i] = cur0;
    int16_t oldsum0 = (cur0<<4);
    while (1) {
		_delay_ms(200);
		// maintain weighted average of 16 readings
		oldsum0 -= olds0[15];
		for (uint8_t i=0; i<15; i++)
			olds0[i+1] = olds0[i];
		cur0 = getADCVal(0);
		olds0[0] = cur0;
		oldsum0 += cur0;
		// show averaged and current readings on the display
		dispVal = cur0<<4;
		_delay_ms(500);
		dispVal = olds0[15]<<4;
		_delay_ms(500);
		// if there's a significant change, an object probably passed in front of the photodiode
		if ((cur0<<4)-oldsum0 > (oldsum0>>5)  || (cur0<<4)-oldsum0 < -(oldsum0>>5)) {
			PORTB |= (1<<0);
		}
		else {
			PORTB &= ~(1<<0);
		}
    }
    return 0;
}
