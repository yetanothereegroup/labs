/*
 * Daniel Ma <dma012@ucr.edu>
 *
 * Lab section: 023
 * Assignment: dma012_kshao006_lab7_part2
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 4/24/2019 23:09:23
 *
 * Author : Daniel Ma <dma012@ucr.edu>
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

#define SM_TICK_TIME 15

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

enum state_t {S_RESET, S_LIT, S_WAIT_DOWN, S_WAIT, S_RESTART_DOWN, S_RESTART, S_VICTORY} state;

void tick(uint8_t button_down, uint8_t *output) {
    static uint32_t cnt;
    static uint8_t score;
    static uint8_t score_prev = (uint8_t)-1;
    static uint8_t ledstate;
    
    // Transitions
    
    switch(state) {
        case S_RESET:
        LCD_ClearScreen();
        cnt = 0;
        score = 0;
        state = S_LIT;
        break;
        
        case S_LIT:
        if (GET_BIT(button_down, 0) == 1) {
            cnt = 0;
            if (ledstate == 0x04) {
                if (score < 9) score++;
            } else {
                if (score >= 1) score--;
            }                
            
            if (score == 9) state = S_VICTORY;
            else state = S_WAIT_DOWN;
        }
        break;
        
        case S_WAIT_DOWN:
        if (GET_BIT(button_down, 0) == 0) {
            state = S_WAIT;
            } else {
            state = S_WAIT_DOWN;
        }
        break;
        
        case S_WAIT:
        if (GET_BIT(button_down, 0) == 1) {
            cnt = 0;
            state = S_RESTART_DOWN;
        }
        break;
        
        case S_RESTART_DOWN:
        if (GET_BIT(button_down, 0) == 0) {
            if (score == 9) state = S_RESET;
            else state = S_RESTART;
        }
        break;
        
        case S_RESTART:
        cnt = 0;
        state = S_LIT;
        break;
        
        case S_VICTORY:
        cnt = 0;
        state = S_WAIT_DOWN;
        break;
        
        default:
        state = S_RESET;
        break;
    }
    
    switch(state) {
        case S_RESET:
        score = 0;
        LCD_Cursor(2);
        LCD_WriteData(score + '0');
        LCD_Cursor(0);
        ledstate = 0x00;
        cnt = 0;
        break;
        
        case S_LIT:
        if (score != score_prev) {
            LCD_Cursor(2);
            LCD_WriteData(score + '0');
            LCD_Cursor(0);
        }
        if (cnt <= 300 / SM_TICK_TIME) {
            ledstate = 0x01;
            cnt++;
            } else if (cnt <= 600 / SM_TICK_TIME) {
            ledstate = 0x02;
            cnt++;
            } else if (cnt <= 900 / SM_TICK_TIME) {
            ledstate = 0x04;
            cnt++;
            } else {
            ledstate = 0x01;
            cnt = 1;
        }
        
        break;
        
        case S_WAIT_DOWN:
        LCD_Cursor(2);
        LCD_WriteData(score + '0');
        LCD_Cursor(0);
        break;
        
        case S_WAIT:
        break;
        
        case S_RESTART_DOWN:
        break;
        
        case S_RESTART:
        ledstate = 0x00;
        cnt = 0;
        break;
        
        case S_VICTORY:
        LCD_DisplayString(1, "You are Winner!!");
        LCD_Cursor(0);
        
        default:
        break;
        
    }
    
    *output = ledstate;
    score_prev = score;
}

int main(void) {
    INITIALISE_PORT(A, 0x00);
    INITIALISE_PORT(B, 0xFF);
    INITIALISE_PORT(C, 0xFF);
    INITIALISE_PORT(D, 0xFF);
    
    LCD_init();
    
    timer_set(SM_TICK_TIME);
    timer_init();
    
    uint8_t buf_A = 0;
    uint8_t buf_B = 0;
    
    while (1) {
        buf_A = (~PINA & 0x01);
        
        tick(buf_A, &buf_B);
        PORTB = buf_B;
        
        while(!timer_flag);
        timer_flag = 0;
    }
}

