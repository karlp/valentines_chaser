// Karl Palsson, 2010
// 
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h> 
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define HEART1          PINB0
#define HEART2          PINB1
#define HEART3          PINB2
#define HEART_ON(x)       (PORTB |= (1<<(x)))
#define HEART_OFF(x)       (PORTB &= ~(1<<(x)))
#define HEART_CONFIG     (DDRB |= (1<<HEART1) | (1<<HEART2) | (1<<HEART3))


void init(void) {
	HEART_CONFIG;
        clock_prescale_set(0);
}


int main(void) {
	init();
        while (1) {
		HEART_ON(HEART1);
		HEART_OFF(HEART3);
		_delay_ms(100);
		HEART_ON(HEART2);
                HEART_OFF(HEART1);
		_delay_ms(100);
		HEART_OFF(HEART2);
                HEART_ON(HEART3);
		_delay_ms(100);
	}
}

