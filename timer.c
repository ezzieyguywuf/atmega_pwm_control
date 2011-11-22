#include <avr/io.h>
#include <util/delay.h>

void timer1Init(void);
void analogInit(void);
int16_t analogGrab(void);

int main(void){
    uint16_t cycle = 0x04;
    CLKPR = 0x80; // Set clock to 'you can change me' mode
    CLKPR = 0x01; // Set clock to 0.5MHz

    // output will be on OC1A
    timer1Init();
    // input will be on PINF1, or ADC1
    analogInit();

    // initialize duty cycle of 0%
    OCR1A = cycle;
    while(1){
        cycle = analogGrab();
        OCR1A = cycle; // >> 2;
        // TODO implement this 'off' check as an interupt
        if (OCR1A < 0x01){
            TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);
        }
        else{
            /*TCNT1 = 0xFF;*/
            TCCR1B |= (1 << CS12 | 1 << CS11);
        }
    }

    return 0;
}

void timer1Init(void){

    // set OC0A to output
    DDRB |= (1 << 7);
    // set timer 0 channel A to CTC mode
    TCCR0A |= (1 << WGM01);
    // set OC0A to toggle at compare match
    TCCR0A |= (1 << COM0A0);
    // maximize compare match to all eight bits
    OCR0A = 0xFF;
    /*// start timer 0 with 1024 pre-scaler*/
    /*TCCR0B |= (1 << CS02 | 1 << CS00);*/
    // start timer 0 with 64 pre-scaler
    TCCR0B |= (1 << CS01 | 1 << CS00);
    /*// start timer 0 with a 256 pre-scaler*/
    /*TCCR0B |= (1 << CS02);*/
    /*// start timer 0 with an 8 pre-scaler*/
    /*TCCR0B |= (1 << CS01);*/

    /*----------------TIMER 1---------------------------------------*/
    // set OC1A to output
    DDRB |= (1 << 5);
    // set T1 to input
    DDRD &= ~(1 << 6);

    // set up timer 1 in fast PWM bode with 10-bit resolution (handy!)
    TCCR1A |= (1 << WGM11 | 1 << WGM10);
    TCCR1B |= (1 << WGM12);
    /*// set up timer 1 in fast PWM bode with 8-bit resolution (handy!)*/
    /*TCCR1A |= (1 << WGM10);*/
    /*TCCR1B |= (1 << WGM12);*/
    // set up output to clear OC1A on compare match, and set it at TOP
    TCCR1A |= (1 << COM1A1);

    // initialize at duty cycle of X%
    OCR1A = 0x0;

    /*// start timer 1 with no prescaling*/
    /*TCCR1B |= (1 << CS10);*/

    /*// start timer 1 with 1024 prescaler*/
    /*TCCR1B |= (1 << CS12 | 1 << CS10);*/

    // TODO start timer 1 from external source, rising edge
    TCCR1B |= (1 << CS12 | 1 << CS11);
}

void analogInit(void){
}

int16_t analogGrab(void){
    uint8_t low = 0;

    DDRF &= ~(1 << 1);            // set PINF1 to input
    ADMUX |= (1 << REFS0);        // use VCC as reference
    ADMUX |= (1 << MUX0);         // set up ADC1 as single ended input
    ADMUX &= ~(1 << ADLAR);       // ensure that output is right-adjusted
                                  // TODO set up Interupt here
    ADCSRA |= (1 << ADEN);        // enable ADC
    ADCSRA |= (1 << ADSC);        // start conversion

    while (ADCSRA & (1 << ADSC)); // wait for conversion to complete

    low = ADCL;
    return ((ADCH << 8) | low);
}
