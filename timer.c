#include <avr/io.h>

static uint8_t aref = (1<<REFS0); // default to AREF = Vcc

int main(void){
    CLKPR = 0x80; // Set clock to 'you can change me' mode
    CLKPR = 0x05; // Set clock to 0.5MHz

    DDRB |= (1 << 5); // set up OC1A as output
    DDRD &= ~(1 << 7); // set up T0 as input
    DDRB |= (1 << 7);  // set up OC0A as output

    TCCR1B |= (1 << CS01); // set up timer1, with a pre-scale of 8
    TCCR1A |= (1 << COM1A0); // set up timer1, channel A in CTC toggle mode 
    OCR1A = 62500; // set up OCR1A=62,500, which equals 1 second

    // set up timer0 to external source, rising edge (input from timer1)
    TCCR0B |= (1 << CS02 | 1 << CS01 | 1 << CS00);
    TCCR0A |= (1 << WGM01 | 1 << WGM00); // set up timer0 to output PWM
    TCCR0A |= (1 << WGM01); // set up timer0 for CTC
    TCCR0A |= (1 << COM0A0 | 1 << COM0A1); // toggle OC0A on compare match

    OCR0A = ~(1<<4) | (1 << 4);// initialize timer B with a 50% duty cycle
    /*OCR0A = 0x08 ;// initialize timer B with a 100% duty cycle*/
    OCR0A = 0x02; // Every two ticks
    while(1){
        // TODO monitor potentiometer
        // TODO change duty cycle based on pot location
        // TODO consider altering range of pot or allowed duty cycles in order
        // to protect our hardware.
    }
    return 0;
}

