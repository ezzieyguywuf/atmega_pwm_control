#include <avr/io.h>

static uint8_t aref = (1<<REFS0); // default to AREF = Vcc

int main(void){
    CLKPR = 0x80; // Set clock to 'you can change me' mode
    CLKPR = 0x05; // Set clock to 0.5MHz
    // TODO set up timer1, with a pre-scale of 8
    // TODO set up timer1, channel A in CTC mode with an OCR1A=62,500, which
    // equals 1 second
    // TODO set up timer0 to external source, rising edge (input from timer1)
    // TODO set up timer0 to output PWM
    while(1){
        // TODO monitor potentiometer
        // TODO change duty cycle based on pot location
        // TODO consider altering range of pot or allowed duty cycles in order
        // to protect our hardware.
    }
    return 0;
}
