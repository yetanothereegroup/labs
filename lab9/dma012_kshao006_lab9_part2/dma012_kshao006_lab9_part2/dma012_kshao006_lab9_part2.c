/*
 * Daniel Ma <dma012@ucr.edu>
 *
 * Lab section: 023
 * Assignment: Lab  Exercise 
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 5/2/2019 08:32:45
 *
 * dma012_kshao006_lab9_part2
 * dma012_kshao006_lab9_part2
 * Author : Daniel Ma <dma012@ucr.edu>
 */ 

#include <avr/io.h>

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }

// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
    static double current_frequency; // Keeps track of the currently set frequency
    // Will only update the registers when the frequency changes, otherwise allows
    // music to play uninterrupted.
    if (frequency != current_frequency) {
        if (!frequency) { TCCR0B &= 0x08; } //stops timer/counter
        else { TCCR0B |= 0x03; } // resumes/continues timer/counter
        
        // prevents OCR3A from overflowing, using prescaler 64
        // 0.954 is smallest frequency that will not result in overflow
        if (frequency < 0.954) { OCR0A = 0xFFFF; }
        
        // prevents OCR0A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
        else if (frequency > 31250) { OCR0A = 0x0000; }
        
        // set OCR3A based on desired frequency
        else { OCR0A = (short)(8000000 / (128 * frequency)) - 1; }

        TCNT0 = 0; // resets counter
        current_frequency = frequency; // Updates the current frequency
    }
}

void PWM_on() {
    TCCR0A = (1 << WGM02) | (1 << WGM00) | (1 << COM0A0);
    // COM3A0: Toggle PB3 on compare match between counter and OCR0A
    TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
    // WGM02: When counter (TCNT0) matches OCR0A, reset counter
    // CS01 & CS30: Set a prescaler of 64
    set_PWM(0);
}

void PWM_off() {
    TCCR0A = 0x00;
    TCCR0B = 0x00;
}

const double notes[] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};

enum adjust_state_type {S_IDLE, S_UP, S_UP_WAIT, S_DOWN, S_DOWN_WAIT, S_RESET} adjust_state;
enum onoff_state_type {S_ON_IDLE, S_ON, S_ON_WAIT, S_OFF_IDLE, S_OFF, S_OFF_WAIT} onoff_state;

void tick(uint8_t button_down) {
    // State transitions
    static uint8_t cnt; // No precision required, just need this to smooth out button presses
    static uint8_t curnote;
    
    switch(onoff_state) {
        case S_ON_IDLE:
        if (GET_BIT(button_down, 1) == 1) {
            onoff_state = S_OFF;
        }
        break;
        
        case S_OFF:
        onoff_state = S_OFF_WAIT;
        break;
        
        case S_OFF_WAIT:
        if (GET_BIT(button_down, 1) == 0) {
            onoff_state = S_OFF_IDLE;
        }
        break;
        
        case S_OFF_IDLE:
        if (GET_BIT(button_down, 1) == 1) {
            onoff_state = S_ON;
        }
        break;
        
        case S_ON:
        onoff_state = S_ON_WAIT;
        break;
        
        case S_ON_WAIT:
        if (GET_BIT(button_down, 1) == 0) {
            onoff_state = S_ON_IDLE;
        }
        break;
        
    }
    
    switch(adjust_state) {
        case S_IDLE:
        if (GET_BIT(button_down, 0)) {
            if (GET_BIT(button_down, 2)) {
                adjust_state = S_RESET;
            } else {
                adjust_state = S_UP;
            }
        } else if (GET_BIT(button_down, 2)) {
            adjust_state = S_DOWN;
        }
        break;
        
        case S_UP:
        adjust_state = S_UP_WAIT;
        break;
        
        case S_UP_WAIT:
        if (GET_BIT(button_down, 0) == 0) {
            adjust_state = S_IDLE;
        }
        break;
        
        case S_DOWN:
        adjust_state = S_DOWN_WAIT;
        break;
        
        case S_DOWN_WAIT:
        if (GET_BIT(button_down, 2) == 0) {
            adjust_state = S_IDLE;
        }
        break;
        
        case S_RESET:
        adjust_state = S_IDLE;
        break;
    }
    
    // State actions
    switch(onoff_state) {
        case S_ON_IDLE:
        break;
        
        case S_OFF:
        PWM_off();
        break;
        
        case S_OFF_WAIT:
        break;
        
        case S_OFF_IDLE:
        break;
        
        case S_ON:
        PWM_on();
        break;
        
        case S_ON_WAIT:
        break;
       
    }
    
    switch(adjust_state) {
        case S_IDLE:
        set_PWM(notes[curnote]);
        break;
        
        case S_UP:
        if (curnote < 7) curnote++;
        set_PWM(notes[curnote]);
        break;
        
        case S_UP_WAIT:
        break;
        
        case S_DOWN:
        if (curnote > 0) curnote--;
        set_PWM(notes[curnote]);
        break;
        
        case S_DOWN_WAIT:
        break;
        
        case S_RESET:
        //curnote = 0;
        //set_PWM(notes[curnote]);
        break;
        
    }
}

int main(void) {
    INITIALISE_PORT(A, 0x00);
    INITIALISE_PORT(B, 0xFF);
    uint8_t buf_a;
    adjust_state = S_RESET;
    onoff_state = S_OFF_IDLE;
    while (1) {
        buf_a = (~PINA) & 0x07;
        
        tick(buf_a);
        
    }
}

