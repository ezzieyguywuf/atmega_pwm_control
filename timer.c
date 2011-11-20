#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include <analog.h>

#define DEBUG
#if defined(DEBUG)
#include "usb_debug_only.h"
#include "print.h"
#endif

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CPU_16MHz       0x00
#define CPU_8MHz        0x01
#define CPU_4MHz        0x02
#define CPU_2MHz        0x03
#define CPU_1MHz        0x04
#define CPU_500kHz      0x05
#define CPU_250kHz      0x06
#define CPU_125kHz      0x07
#define CPU_62kHz       0x08

static uint8_t aref = (1<<REFS0); // default to AREF = Vcc

int main(void){
    CPU_PRESCALE(CPU_2MHz); // Note that I've used the 1MHz clock speed in order
                            // to agree with the expamples in the pdf
#if defined(DEBUG)
    usb_init();
#endif
    DDRB |= (1 << 5); // Set Port B, pin 5 to output, as this correspondse to OC1A

    // TODO set 8-bit timer to CTC  mode 
    // TODO set count of timer 4 channel A to the equivalent of 1Hz


    // TODO start timer 4 at a prescale value of (64?)

    int last=-1, curr=0,i;
    curr = analogRead(PINF1);
    OCR1A = curr;

#if defined(DEBUG)
    int change=1;
    char outStr[16];
#endif

    while(1){
        // read in value from Pot. This is a 10-bit value betweer 0 and ~(1<<10)
        curr = analogRead(PINF1);
        
        // check if the pot has moved a pre-set amount, to reduce noise
        // TODO check to see if a 'better' pot can be obtained/used
        // 0x05 = xx00 0000 0011
        // Effectively we're only using 8-bits from the pot: let's use the 8-bit
        // timer then!
        if (abs(curr - last) > 0x03){
            // TODO update timer counter or duty cycle register with upper 8
            // bits of the pot reading
#if defined(DEBUG)
            // TODO implement debug messages as an interrupt to clean up code
            change = 1;
            if (change == 1){
                print("Pot setting = ");
                /*itoa(percentage, outStr, 10);*/
                /*usb_debug_putstr(outStr, 3);*/
                /*print("  phex16 = 0x");*/
                phex16(OCR1A);
                print("\n");
                change = 0;
            }
#endif
        last = curr;
        }
    }
    return 0;
}
void analogReference(uint8_t mode)
{
	aref = mode & 0xC0;
}


// Arduino compatible pin input
int16_t analogRead(uint8_t pin)
{
	static const uint8_t PROGMEM pin_to_mux[] = {
		0x00, 0x01, 0x04, 0x05, 0x06, 0x07,
		0x25, 0x24, 0x23, 0x22, 0x21, 0x20};
	if (pin >= 12) return 0;
	return adc_read(pgm_read_byte(pin_to_mux + pin));
}

// Mux input
int16_t adc_read(uint8_t mux)
{
	uint8_t low;

	ADCSRA = (1<<ADEN) | ADC_PRESCALER;		// enable ADC
	ADCSRB = (1<<ADHSM) | (mux & 0x20);		// high speed mode
	ADMUX = aref | (mux & 0x1F);			// configure mux input
	ADCSRA = (1<<ADEN) | ADC_PRESCALER | (1<<ADSC);	// start the conversion
	while (ADCSRA & (1<<ADSC)) ;			// wait for result
	low = ADCL;					// must read LSB first
	return (ADCH << 8) | low;			// must read MSB only once!
}
