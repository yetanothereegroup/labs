/*
 * Daniel Ma <dma012@ucr.edu>
 *
 *
 * Lab section: 023
 * Assignment: dma012_kshao009_lab10_part1
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 5/6/2019 22:45:18
 *
 */ 

#include <avr/io.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include "common.h"

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)

#define SET_BIT(var, bit, val) { \
    if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); \
}

typedef struct _task {
    uint8_t state;
    // Period, in ms
    uint32_t period;
    uint32_t elapsed_time;
    uint32_t (*tick)(uint32_t);
} task;

#define TASK_SIZE 5
task task_list[TASK_SIZE];


enum _state_TLED {S_TLED_1, S_TLED_2, S_TLED_3};
enum _state_BLINK {S_BLINK_ON, S_BLINK_OFF};
enum _state_BUZZER {S_B4_ON, S_B4_OFF};
enum _state_FREQSET {S_FREQSET_IDLE, S_FREQSET_UP, S_FREQSET_UP_WAIT, S_FREQSET_DOWN, S_FREQSET_DOWN_WAIT};
    
uint8_t B0, B1, B2, B3, B4;
uint8_t A0, A1, A2, A3, A4;

uint8_t frequency = 10; // Not actual frequency in Hz.  Length of period in ms.
    
uint32_t tick_TLED (uint32_t state) {
    // Handle state transitions
    switch (state) {
        case S_TLED_1:
        state = S_TLED_2;
        break;
        case S_TLED_2:
        state = S_TLED_3;
        break;
        case S_TLED_3:
        state = S_TLED_1;
        break;
        default:
        state = S_TLED_1;
        break;
    }
    switch(state) {
        case S_TLED_1:
        B0 = 1;
        B1 = B2 = 0;
        break;
        case S_TLED_2:
        B1 = 1;
        B0 = B2 = 0;
        break;
        case S_TLED_3:
        B2 = 1;
        B0 = B1 = 0;
        break;
    }
    return state;
}

uint32_t tick_BLINK (uint32_t state) {
    switch(state) {
        case S_BLINK_ON:
        state = S_BLINK_OFF;
        break;
        case S_BLINK_OFF:
        state = S_BLINK_ON;
        break;
        default:
        state = S_BLINK_ON;
        break;
    }
    
    switch(state) {
        case S_BLINK_ON:
        B3 = 1;
        break;
        case S_BLINK_OFF:
        B3 = 0;
        break;
    }
    return state;
}

uint32_t tick_OUTPUT(uint32_t unused) {
    uint8_t b_buf = 0;
    SET_BIT(b_buf, 0, B0);
    SET_BIT(b_buf, 1, B1);
    SET_BIT(b_buf, 2, B2);
    SET_BIT(b_buf, 3, B3);
    SET_BIT(b_buf, 4, B4);
    
    uint8_t a_buf = PINA;
    
    A0 = (GET_BIT(a_buf, 0)) ? 0 : 1;
    A1 = (GET_BIT(a_buf, 1)) ? 0 : 1;
    A2 = (GET_BIT(a_buf, 2)) ? 0 : 1;
    A3 = (GET_BIT(a_buf, 3)) ? 0 : 1;
    A4 = (GET_BIT(a_buf, 4)) ? 0 : 1;
    
    
    
    PORTB = b_buf;
    
    return 0;
    
}

uint32_t tick_BUZZER(uint32_t state) {
    static uint8_t cnt;
    switch(state) {
        case S_B4_ON:
        cnt++;
        if (cnt >= frequency / 2) state = S_B4_OFF;
        break;
        case S_B4_OFF:
        cnt++;
        if (cnt >= frequency) {
            state = S_B4_ON;
            cnt = 0;
        }            
        break;
        
        default:
        state = S_B4_ON;
        break;
    }
    
    switch(state) {
        case S_B4_ON:
        if (GET_BIT(PINA, 2) == 0) B4 = 1;
        break;
        case S_B4_OFF:
        B4 = 0;
        break;
    }
    
    return state;
}

uint32_t tick_FREQSET(uint32_t state) {
    switch(state) {
        case S_FREQSET_IDLE:
        if (A0) state = S_FREQSET_UP;
        else if (A1) state = S_FREQSET_DOWN;
        break;
        
        case S_FREQSET_UP:
        state = S_FREQSET_UP_WAIT;
        break;
        
        case S_FREQSET_UP_WAIT:
        if (!A0) state = S_FREQSET_IDLE;
        break;
        
        case S_FREQSET_DOWN:
        state = S_FREQSET_DOWN_WAIT;
        break;
        case S_FREQSET_DOWN_WAIT:
        if (!A1) state = S_FREQSET_IDLE;
        break;
        default:
        state = S_FREQSET_IDLE;
        break;
        
    }
    
    switch(state) {
        case S_FREQSET_IDLE:
        break;
        case S_FREQSET_UP:
        if (frequency < 127) frequency++;
        break;
        case S_FREQSET_UP_WAIT:
        break;
        case S_FREQSET_DOWN:
        if (frequency > 0) frequency--;
        break;
        case S_FREQSET_DOWN_WAIT:
        break;
        default:
        break;
    }
    
    return state;
}

void timer_ISR() {
    for (uint32_t i = 0; i < TASK_SIZE; i++) {
        task_list[i].elapsed_time++;
        if (task_list[i].elapsed_time >= task_list[i].period) {
            task_list[i].elapsed_time = 0;
            uint32_t tmp = (*task_list[i].tick)(task_list[i].state);
            task_list[i].state = tmp;
            
        }
    }
}

int main(void) {
    INITIALISE_PORT(B, 0xFF);
    INITIALISE_PORT(A, 0x00);
    // TLED
    task_list[0].elapsed_time = 0;
    task_list[0].period = 300;
    task_list[0].state = S_TLED_1;
    task_list[0].tick = &tick_TLED;
    
    task_list[1].elapsed_time = 0;
    task_list[1].period = 1000;
    task_list[1].state = S_BLINK_ON;
    task_list[1].tick = &tick_BLINK;
    
    task_list[2].elapsed_time = 0;
    task_list[2].period = 1;
    task_list[2].state = 0;
    task_list[2].tick = &tick_OUTPUT;
    
    task_list[3].elapsed_time = 0;
    task_list[3].period = 1;
    task_list[3].state = S_B4_ON;
    task_list[3].tick = &tick_BUZZER;
    
    task_list[4].elapsed_time = 0;
    task_list[4].period = 1;
    task_list[4].state = S_FREQSET_IDLE;
    task_list[4].tick = &tick_FREQSET;
    
    
    timer_init();
    timer_set(1);
    
    
    while (1) {
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }
}

