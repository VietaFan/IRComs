/*ircomslib.c - Sam Heil 24.01.16
Library for infrared communication

Place photodiode on PORTC, IR_IN_PIN.
Place IR LED on IR_OUT_PORT, IR_OUT_PIN.

sendByte(uint8_t val) sends a byte
getByte() -> uint8_t gets a byte

Change IR_DELAY_MS and IR_DELAY to change
bit-rate.

Uses TIMER2 for getByte.
*/
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h> 

#define IR_OUT_PORT PORTD
#define IR_OUT_PIN PD3
#define IR_IN_PIN PC0
#define IR_OUT_DDR DDRD

#define IR_DELAY_MS
#define IR_DELAY 10
#define IR_DELAY_BIG 1000*IR_DELAY

#define THRESHOLD_ITERS 32
#define THRESHOLD_DELAY 500
#define THRESHOLD_RANGE_MULT 21

#define F_CPU 16000000L

volatile uint16_t ir_threshold;

inline uint16_t getADCVal() {
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));	
	return ADC;
}

uint16_t getThreshold() {
	uint16_t min_val = 0x400, max_val = 0, x;
	IR_OUT_PORT |= (1<<IR_OUT_PIN);
	uint8_t i;
	for (i=0; i<THRESHOLD_ITERS; i++) {
		x = getADCVal();
		_delay_us(THRESHOLD_DELAY);
		if (x < min_val)
			min_val = x;
		if (x > max_val)
			max_val = x;
	}
	IR_OUT_PORT &= ~(1<<IR_OUT_PIN);
	return max_val+(((max_val-min_val)*THRESHOLD_RANGE_MULT)>>6);
}

uint8_t nextChunkSize(uint16_t stopVal) {
  uint8_t ans = 1;
  uint8_t on = getADCVal() > ir_threshold;
  _delay_ms(IR_DELAY/2);
  if ((getADCVal()>ir_threshold) != on)
    return 0;
  for (;((getADCVal()>ir_threshold) == on) && ans<stopVal; ans++)
    _delay_ms(IR_DELAY);
  return ans;
}

inline void irDelay() {
	#ifdef IR_DELAY_MS
	_delay_ms(IR_DELAY);
	#else
	_delay_us(IR_DELAY);
	#endif
}
uint8_t getByte() {
	uint8_t ans=0,stop=9,one=0,k,pos=0,i;
	while (getADCVal()<ir_threshold);
	k = nextChunkSize(stop);
	ans = ((1<<(k-1))-1);
	stop -= k;
	while (stop) {
		k = nextChunkSize(stop);
		if (one) 
			ans += (((1<<k)-1)<<(8-stop));
		stop -= k;
		one = !one;
	}
	return ans;
}

void sendByte(uint8_t val) {
	uint8_t i=0;
	IR_OUT_PORT &= ~(1<<IR_OUT_PIN);
	irDelay();
	IR_OUT_PORT |= (1<<IR_OUT_PIN);
	irDelay();
	while (i < 8) {
		if (val&1)
			IR_OUT_PORT |= (1<<IR_OUT_PIN);
		else
			IR_OUT_PORT &= ~(1<<IR_OUT_PIN);
		val >>= 1;
		i++;
		irDelay();
	}
	IR_OUT_PORT &= ~(1<<IR_OUT_PIN);
        irDelay();
}

inline void initIRComs() {
	/*init ADC for photodiode in*/
	ADMUX = IR_IN_PIN;
	ADCSRA |= (1<<ADPS1) | (1<<ADPS0);
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADEN);
	/*init IR LED output pin*/
	IR_OUT_DDR |= (1<<IR_OUT_PIN);
	/*get IR threshold value*/
	ir_threshold = getThreshold();
	/*init timer for receiving byte*/
	TCCR2B |= (1<<CS20) | (1<<CS21); /* /32 prescaler*/	
}

