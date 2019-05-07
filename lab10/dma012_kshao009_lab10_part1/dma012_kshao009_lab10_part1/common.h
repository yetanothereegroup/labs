/*
 * common.h
 *
 * Created: 5/6/2019 22:28:35
 *  Author: danie
 */ 


#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>
#include <avr/interrupt.h>

// Timer

volatile unsigned char TimerFlag = 0; 

unsigned long avr_timer_max = 1; 
unsigned long avr_timer_current = 0; 

void timer_init() {
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

    //Initialize avr counter
    TCNT1=0;
    
    avr_timer_current = avr_timer_max;
    sei();
}

void timer_off() {
    TCCR1B = 0x00; 
}

void timer_ISR() {
    TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
    // CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
    avr_timer_current--; // Count down to 0 rather than up to TOP
    if (avr_timer_current == 0) { // results in a more efficient compare
        timer_ISR(); // Call the ISR that the user uses
        avr_timer_current = avr_timer_max;
    }
}

// Set TimerISR() to tick every M ms
void timer_set(unsigned long M) {
    avr_timer_max = M;
    avr_timer_current = avr_timer_max;
}

// PWM
// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
    static double current_frequency; // Keeps track of the currently set frequency
    // Will only update the registers when the frequency changes, otherwise allows
    // music to play uninterrupted.
    if (frequency != current_frequency) {
        if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
        else { TCCR3B |= 0x03; } // resumes/continues timer/counter
        
        // prevents OCR3A from overflowing, using prescaler 64
        // 0.954 is smallest frequency that will not result in overflow
        if (frequency < 0.954) { OCR3A = 0xFFFF; }
        
        // prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
        else if (frequency > 31250) { OCR3A = 0x0000; }
        
        // set OCR3A based on desired frequency
        else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

        TCNT3 = 0; // resets counter
        current_frequency = frequency; // Updates the current frequency
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    // COM3A0: Toggle PB6 on compare match between counter and OCR3A
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    // WGM32: When counter (TCNT3) matches OCR3A, reset counter
    // CS31 & CS30: Set a prescaler of 64
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}



#endif /* COMMON_H_ */