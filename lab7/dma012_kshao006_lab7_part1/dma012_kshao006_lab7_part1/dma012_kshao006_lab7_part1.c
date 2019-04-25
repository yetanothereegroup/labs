/*
 * Daniel Ma <dma012@ucr.edu>
 * Ken Shao <kshao006@ucr.edu>
 *
 * Lab section: 023
 * Assignment: dma012_kshao006_lab7_part1
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 4/24/2019 23:09:23
 *
 * Author : Daniel Ma <dma012@ucr.edu>
 *          Ken Shao <kshao006@ucr.edu>
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }

#define SM_TICK_TIME 20

volatile unsigned char timer_flag = 0;

uint32_t avr_timer_M_ = 1;
uint32_t avr_timer_cnt_current_ = 0;
void timer_init() {
    TCCR1B = 0x0B;
    OCR1A = 125;
    
    TIMSK1 = 0x02;
    
    TCNT1 = 0;
    avr_timer_cnt_current_ = avr_timer_M_;
    
    sei();
    
}

void timer_set(uint32_t M) {
    avr_timer_M_ = M;
    avr_timer_cnt_current_ = avr_timer_M_;
}

void timer_off() {
    TCCR1B = 0x00;   
}

void timer_ISR() {
    timer_flag = 1;
}

ISR(TIMER1_COMPA_vect) {
    avr_timer_cnt_current_--;
    if (avr_timer_cnt_current_ == 0) {
        timer_ISR();
        avr_timer_cnt_current_ = avr_timer_M_;
    }
}

enum sm_state_t {S_RESET, S_IDLE, S_INC_DOWN, S_INC, S_DEC_DOWN, S_DEC} counter_state;
void tick(uint8_t button_down, uint8_t* number) {
    static uint32_t cnt;
    // State transitions
    switch(counter_state) {
        // Wait until all buttons up for reset to finish
        case S_RESET:
        cnt = 0;
        if (GET_BIT(button_down, 0) || GET_BIT(button_down, 1)) {}
        else {counter_state = S_IDLE;}
        break;
        
        case S_IDLE:
        cnt = 0;
        if (GET_BIT(button_down, 0)) {
            if (GET_BIT(button_down, 1))
                counter_state = S_RESET;
            else
                counter_state = S_INC_DOWN;
            } else if (GET_BIT(button_down, 1)) { // We already know 0 is not depressed; don't give counseling services
            counter_state = S_DEC_DOWN;
        }
        break;
        
        case S_INC_DOWN:
        cnt = 0;
        counter_state = S_INC;
        break;
        
        case S_INC:
        if (!GET_BIT(button_down, 0)) counter_state = S_IDLE;
        else if (GET_BIT(button_down, 1)) counter_state = S_RESET;
        break;
        
        case S_DEC_DOWN:
        cnt = 0;
        counter_state = S_DEC;
        break;
        
        case S_DEC:
        if (!GET_BIT(button_down, 1)) counter_state = S_IDLE;
        else if (GET_BIT(button_down, 0)) counter_state = S_RESET;
        break;
        
        default:
        counter_state = S_RESET;
        
    }
    
    // We want to do the action on button down.
    // State actions
    switch(counter_state) {
        case S_RESET:
        *number = 0;
        break;
        
        case S_IDLE:
        break;
        
        case S_INC_DOWN:
        if (*number < 9) (*number)++;
        break;
        
        case S_INC:
        cnt++;
        if (cnt >= 1000 / SM_TICK_TIME) {
            if (*number < 9) (*number)++;
            cnt = 0;
        }
        break;
        
        case S_DEC_DOWN:
        if (*number >= 1) (*number)--;
        break;
        
        case S_DEC:
        cnt++;
        if (cnt >= 1000 / SM_TICK_TIME) {
            if (*number >= 1) (*number)--;
            cnt = 0;
        }
        break;
        
        default:
        break;
    }
    
}

int main(void) {
    timer_init();
    timer_set(SM_TICK_TIME);
    INITIALISE_PORT(A, 0x00);
    INITIALISE_PORT(C, 0xFF);
    INITIALISE_PORT(D, 0xFF);
    
    LCD_init();
    LCD_Cursor(2);
    
    uint8_t number;
    uint8_t a_buf;
    uint8_t prev_num = (uint8_t)-1;
    while (1) {
        a_buf = ~PINA & 0x03;
        
        
        tick(a_buf, &number);   
        
        if (number != prev_num) {
            LCD_Cursor(2);
            LCD_WriteData(number + '0');
            LCD_Cursor(0);
        }        
        
        prev_num = number;
        while(!timer_flag);
        timer_flag = 0;
    }
    return 0;
}

