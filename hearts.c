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
#define BUTTON          PINA7

#define ADC_ENABLE  (ADCSRA |= (1<<ADEN))
#define ADC_DISABLE  (ADCSRA &= ~(1<<ADEN))
#define VREF_VCC  0

#define DEBOUNCE 200  // 200Hz debounce clock

unsigned int delay = 100;
static volatile uint8_t button_state;

void init_adc_regular(uint8_t muxBits) {
    ADMUX = VREF_VCC | muxBits;
}

unsigned int adc_read(void)
{
    ADCSRA |= (1<<ADSC);               // begin the conversion
    while (ADCSRA & (1<<ADSC)) ;   // wait for the conversion to complete
    unsigned char lsb = ADCL;                              // read the LSB first
    return (ADCH << 8) | lsb;          // read the MSB and return 10 bit result
}


void init(void) {
    HEART_CONFIG;
    clock_prescale_set(0);
    ADC_ENABLE;
    DDRA &= ~(1<<BUTTON); // input
    DDRA |= (1<<PINA2);  // output for the state indicator led
    PORTA |= (1<<BUTTON); // Pull up please
    
    TCCR1B = (1<<WGM12) | (1<<CS10);  // clear timer on compare match, no prescaler
    OCR1A  =  F_CPU/DEBOUNCE;         // timer = 5 msec  this is 40k, hence needing the 16 bit timer
    TIMSK1  = (1<<OCIE1A);            // enable Output Compare 1 overflow interrupt
}

void play_pattern_1() {
    HEART_ON(HEART1);
    HEART_OFF(HEART3);
    _delay_ms(delay);
    HEART_ON(HEART2);
    HEART_OFF(HEART1);
    _delay_ms(delay);
    HEART_OFF(HEART2);
    HEART_ON(HEART3);
    _delay_ms(delay);
}

void play_pattern_2() {
    HEART_ON(HEART1);
    HEART_OFF(HEART2);
    HEART_ON(HEART3);
    _delay_ms(delay);
    HEART_ON(HEART1);
    HEART_ON(HEART2);
    HEART_OFF(HEART3);
    _delay_ms(delay);
    HEART_OFF(HEART1);
    HEART_ON(HEART2);
    HEART_ON(HEART3);
    _delay_ms(delay);
}

ISR(SIG_OUTPUT_COMPARE1A) {
    static int count = 0;
    static int last = 0;
    int real = (PORTA & PINA7);
    if (real) {
        real = 1;
    } else {
        real = 0;
    }
    // button state gets a zero if the button is pressed.
    if (last ^ real) {
        count = 0;
    } else {
        count++;
    }
    if (count > 4) {
        button_state = 1;
    }
    last = real;
}

// scale the adc reading to 0ms-500ms (ish)
unsigned int read_scaled_pot() {
    init_adc_regular(0);
    unsigned int tmp = adc_read();
    return tmp >> 1;
}

int is_button_pressed() {
    if (button_state) {
        //button_state = 0;
        return 1;
    } 
    return 0;
}

int main(void) {
    init();
    char state = 1;
    while (1) {
        delay = read_scaled_pot();
        if (is_button_pressed()) {
            state++;
            // exciting, only two patterns :)
            if (state > 2) {
                state = 1;
            }
        }
        switch(state) {
        case 1:
            PORTA |= (1<<PINA2);
            play_pattern_1(); 
            break;
        case 2:
            PORTA &= ~(1<<PINA2);
            play_pattern_2();
            break;
        default: play_pattern_1();
        }
    }
}

