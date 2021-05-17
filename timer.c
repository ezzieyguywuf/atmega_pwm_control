#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void timer1Init(void);
void analogInit(void);

int main(void){
    // Note: I came back to this code after years and realized I didn't understand what
    // most of it meant. So, here I will capture some notes that will hopefully help.
    //
    // - A 'timer' is something that counts ticks of the system clock and does something
    // with that count.
    // http://extremeelectronics.co.in/avr-tutorials/avr-timers-an-introduction/ has a
    // pretty good summary.
    // - Timer's are handy b/c they increment/decrement or w/e without the CPU needing to
    // do anything: it goes on in the background. This allows you to free up logic for
    // other things.
    // - When timer overflows, an interrupt can be issued and an Interupt Service Routine
    // (ISR) written to 'do stuff' when that interrupt occurs.
    // - a 'prescaler' divides the system clock by a certain value. This is the clock that
    // the timer runs on. a prescaler of 64 on a system clock of 0.5 MHz runs at 0.5/64
    // MHz = 7.8125 KHz
    // - TCCR0A is the _control register_ for OCR0A behaviour. Each of the eight bits in
    // TCCR0A controls a different setting. Each of these bits can be set 'manually', i.e.
    // "1 << 3" or, preferably, using pre-defined macros, i.e. "1 << WGM01". Refer to the
    // datasheet for more info on control register bits.
    // - There are two timers on this controller: 8-bit timer 0 and 16 bit timer1 and
    // timer3. Oh, also 10-bit timer4.
    // - We are only using timer0
    // - Timer0 has to "output compare registers", OCR0A and OCR0B. Be are using only one
    // of these
    // - In the datasheet, BITXX:X=Y, i.e. WGM02:0 = 2, means "WGM02, WGM01, WGM00 = 2"
    // where each of the three bits corresponds to a bit in a whole 'word'. In other wors,
    // WGM02 = bit3, WGM01 = bit2, WGM00 = bit1, and for WGM02:0=2, WGM00=0, WGM01=1,
    // WGM02=0.
    // HERE IS A GENERAL SUMMARY:
    // 1) OC0A is physical port PB7 on the teensy. It gets toggled by timer 0
    // 2) T1 can recieve input on physical port PD6 on the teensy. I have PB7 wired to PD6
    // (I believe) so that Timer 0's output is Timer 1's input
    // 3) This means that timer 1 'ticks' every times timer 0 overflows. With system clock
    // set to 0.5 MHz and a prescaler of 64 on timer 0 and 8 bit overflow value of 255,
    // timer 1 should be ticking at approximatel 30 Hz, or every 0.03 seconds TODO: is
    // this math right?
    // 4)The MATCH on timer 1 is set by the analog input ADC from the knob. When timer
    // count reaches MATCH, OC1A is turned off. When timer count overflows, OC1A is turned
    // on
    // 5) OC1A is PB5 on the teensy. I have this wired to the SSR
    // 6) Since timer 2 is on a 10-bit register, this means it is overflowing at a rate of
    // approx 30 Hz / 0x3FF or approx 0.029 hz. 
    CLKPR = 0x80; // Set clock to 'you can change me' mode
    CLKPR = 0x01; // Set clock to 0.5MHz


    timer1Init(); // Initialize timers needed. output will be on OC1A
    analogInit(); // Initialize ADC. input will be on PINF1, or ADC1

    while(1){
    }

    return 0;
}

void timer1Init(void){

    /*----------------TIMER 0---------------------------------------*/
    // The pin associated with OC0A must be set to 'output' in order for COM0A0 to work.
    // What I mean by that is that if you want the output compare pin to toggle, clear, or
    // set the pin on MATCH, this must be set to out.  See page 102 in the data sheet. DDR
    // stands for Data Direction Register. OC0A is on port B, number 7 (PB7, see
    // https://www.pjrc.com/teensy/pinout.html), therefore we have to set the seventh bit
    // on DDRB to 1 for 'output'.
    DDRB |= (1 << 7);

    // --------------------
    // Note: The mode of operation is determined by _both_ the Waveform Generation mode
    // (WGM) and tho Compare Output mode (COM) bits
    // --------------------
    // WGM02:0=2 sets timer 0 to Clear Timer on Compare (CTC) mode. See table on page 104 of
    // datasheet. Between WGM00, WGM01 (both on TCCR0A) and WGM02 (TCCR0B), 6 different
    // operating modes can be chosen. In CTC mode, the counter is cleared to 0 when the
    // counter value (TCNT0) equals the OCR0A, i.e. the Overflow Count Register value.
    TCCR0A |= (1 << WGM01);
    // COM0A1:0=1 sets OC0A (remember we set that as the output signal earlier?) to
    // 'toggle' mode. Any time TCNT0==OCR0A, the output pin will toggle between on and off
    TCCR0A |= (1 << COM0A0);

    // Set the overflow value. Since this is an 8-bit timer, the max value is 0xFF
    /*OCR0A = 0X80;*/
    OCR0A = 0xFF;

    // Setting CS02:0=3 (i.e. CS01 and CS00 to 1) is the max prescaler available on Timer
    // 0. The prescaler value is FCPU/64, i.e. system/64
    TCCR0B |= (1 << CS01 | 1 << CS00);

    /*----------------TIMER 1---------------------------------------*/
    // Set pin associated with OC1A to 'output'. This pin is on port B number 5, or PB5. I
    // have this wired to turn the SSR on/off
    DDRB |= (1 << 5);
    // Set the pin associated with timer1, which is on port D number 6, to input. This
    // means that it will use an external source for the timer. I believe that I have
    // wired it such that the output from timer 0 is the input for timer 1.
    DDRD &= ~(1 << 6);

    // WGM12:0=7 means 'fast pwm mode, 10-bit'. Since the ADC has 10-bit resolution, this
    // should allow directly setting the upper limit to the ADC value.
    TCCR1A |= (1 << WGM11 | 1 << WGM10);
    TCCR1B |= (1 << WGM12);

    // Per table 14-3 on page 130 of the datasheet, when in fast pwm mode, having
    // COM1A1:0=2 means "clear OC1A/OC1B/OC1C on match, set at TOP". This is where the
    // on/off functionality comes from. It always get's turned on at overflow, then get's
    // turned off when the MATCH occurs. The MATCH target changes based on the ADC value.
    TCCR1A |= (1 << COM1A1);

    // initialize at a duty cycle of 0%
    OCR1A = 0x00;

    // This sets the clock source to external "on Tn pin. Clock on falling edge"
    TCCR1B |= (1 << CS12 | 1 << CS11);
}

void analogInit(void){
    // TODO enable high speed mode? Check if this is needed

    // Set PF1 to input on the teensy. This is where the knob is wired to
    DDRF &= ~(1 << 1);
    // VCC is used as reference. I'm supposed to use a capacitor, not user if I did...
    ADMUX |= (1 << REFS0);
    // Set up ADC1 as single-ended input. This is PF1 on the teensy.
    ADMUX |= (1 << MUX0);
    // Ensure that output is right-adjusted
    ADMUX &= ~(1 << ADLAR);

    // This sets the Auto Trigger. This allows a change in the analog signal to re-start
    // the ADC conversion
    ADCSRA |= (1 << ADATE);
    /*ADCSRB |= (1 << ADHSM);       // Enable high-speed mode*/
                                  // TODO Disable non-used analog inputs for
                                  // power savings?
    // 128 pre-scaler
    ADCSRA |= (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);
    // Enable no trigger source, i.e. free-running mode
    ADCSRB &= ~(1 << ADTS3 | 1 << ADTS2 | 1 << ADTS1 | 1 << ADTS0);
    // ADC interupt enable
    ADCSRA |= (1 << ADIE);
    // This sets the global interrupt flag
    sei();
    // enable ADC
    ADCSRA |= (1 << ADEN);
    // start the conversion
    ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect){
    uint8_t low = 0;
    // must be read in two parts
    low = ADCL;
    uint16_t full = (ADCH << 8) | low;

    // Convert value so that we keep the duty cycle between 50% and 100%. Since OCR1A is
    // 10 bits, that means the max is 0x3FF or 1023. 50% of that is 511.5. Let's say 512,
    // which is 0x200. Let's do some math
    // 1 * x = 512
    // 2 * x = 512
    // 3 * x = 513
    // 4 * x = 513
    // ACT/2 + 512 maybe?
    full = full/2 + 512.;

    // OCR1A is the MATCH value. The higher, the longer the SSR stays ON.
    OCR1A = full;
}
