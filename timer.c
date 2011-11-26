#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void timer1Init(void);
void analogInit(void);

int main(void){
    CLKPR = 0x80; // Set clock to 'you can change me' mode
    CLKPR = 0x01; // Set clock to 0.5MHz


    timer1Init(); // Initialize timers needed. output will be on OC1A
    analogInit(); // Initialize ADC. input will be on PINF1, or ADC1

    while(1){
    }

    return 0;
}

void timer1Init(void){

    DDRB |= (1 << 7);                    // set OC0A to output
    TCCR0A |= (1 << WGM01);              // set timer 0 channel A to CTC mode
    TCCR0A |= (1 << COM0A0);             // set OC0A to toggle at compare match
    OCR0A = 0xFF;                        // maximize compare match to all eight bits
    /*TCCR0B |= (1 << CS02 | 1 << CS00); // start timer 0 with 1024 pre-scaler*/
    TCCR0B |= (1 << CS01 | 1 << CS00); // start timer 0 with 64 pre-scaler
    /*TCCR0B |= (1 << CS02);               // start timer 0 with a 256 pre-scaler*/
    /*TCCR0B |= (1 << CS01);             // start timer 0 with an 8 pre-scaler*/

    /*----------------TIMER 1---------------------------------------*/
    DDRB |= (1 << 5);                       // set OC1A to output
    DDRD &= ~(1 << 6);                      // set T1 to input

    TCCR1A |= (1 << WGM11 | 1 << WGM10);    // set up timer 1 in fast PWM bode with
    TCCR1B |= (1 << WGM12);                 // 10-bit resolution (handy!)
    /*TCCR1A |= (1 << WGM10);               // set up timer 1 in fast PWM bode with 8-bit*/
    /*TCCR1B |= (1 << WGM12);               // resolution (handy!)*/

    TCCR1A |= (1 << COM1A1);                // set up output to clear OC1A on compare match,
                                            // and set it at TOP

    OCR1A = 0x4F;                            // initialize at duty cycle of 50%

    /*TCCR1B |= (1 << CS10);                // start timer 1 with no prescaling*/
    /*TCCR1B |= (1 << CS12 | 1 << CS10);    // start timer 1 with 1024 prescaler*/

    TCCR1B |= (1 << CS12 | 1 << CS11);      // start timer 1 from external
                                            // source, rising edge
}

void analogInit(void){
    // TODO enable high speed mode? Check if this is needed

    DDRF &= ~(1 << 1);            // set PINF1 to input
    ADMUX |= (1 << REFS0);        // use VCC as reference
    ADMUX |= (1 << MUX0);         // set up ADC1 as single ended input
    ADMUX &= ~(1 << ADLAR);       // ensure that output is right-adjusted

    ADCSRA |= (1 << ADATE);       // Auto Trigger Enable, so that the interupt
                                  // can re-start the conversion
    /*ADCSRB |= (1 << ADHSM);       // Enable high-speed mode*/
                                  // TODO Disable non-used analog inputs for
                                  // power savings?
    // 128 pre-scaler
    ADCSRA |= (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);
    // Enable no trigger source, i.e. free-running mode
    ADCSRB &= ~(1 << ADTS3 | 1 << ADTS2 | 1 << ADTS1 | 1 << ADTS0);
    ADCSRA |= (1 << ADIE);        // ADC Interupt Enable
    sei();
    ADCSRA |= (1 << ADEN);        // enable ADC
    ADCSRA |= (1 << ADSC); //start conversion
}

ISR(ADC_vect){
    uint8_t low = 0;
    low = ADCL;
    OCR1A = (ADCH << 8) | low;
}
