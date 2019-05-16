/*
 * Daniel Ma <dma012@ucr.edu>
 *
 * Lab section: 023
 * Assignment: Lab  Exercise 
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 5/14/2019 08:21:52
 *
 * dma012_kshao009_lab11_part1
 * dma012_kshao009_lab11_part1
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

typedef enum _task_flag {TF_ENABLED = 1, TF_REPEAT = 1 << 1} task_flag;

typedef struct _task {
    uint8_t state;
    task_flag flags;
    uint32_t period;
    uint32_t elapsed_time;
    uint32_t (*tick)(uint32_t);
} task;

#define TASK_SIZE 1;
task tasks[TASK_SIZE]; 


void timer_ISR() {
    for (uint32_t i = 0; i < TASK_SIZE; i++) {
        tasks[i].elapsed_time++;
        if (tasks[i].elapsed_time >= tasks[i].period) {
            tasks[i].elapsed_time = 0;
            tasks[i].state = *(tasks[i].tick)(tasks[i].state);
        }
    }
}


uint8_t A0, A1, A2, A3
uint32_t tick_IO(uint32_t unused) {
    
}

int main(void) {
    
    
    while (1) {
    }
}

