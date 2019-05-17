/*
 * Daniel Ma <dma012@ucr.edu>
 *
 * Lab section: 023
 * Assignment: Lab  Exercise 
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 5/16/2019 08:12:09
 *
 * dma012_kshao006_lab11_part5
 * dma012_kshao006_lab11_part5
 * Author : Daniel Ma <dma012@ucr.edu>
 */ 

#include <avr/io.h>
#include "lcd.h"
#include "common.h"

typedef enum _task_flag {TF_ENABLE = 1, TF_REPEAT = 1 << 1} task_flag;

typedef struct _task {
    uint8_t state;
    task_flag flags;
    uint32_t period;
    uint32_t elapsed_time;
    uint32_t (*tick)(uint32_t);
} task;

#define D_TASK_MAX 8
uint32_t num_tasks = 0;


task tasks[D_TASK_MAX];
void add_task(uint32_t period, uint32_t (*tick)(uint32_t), task_flag flags) {
    tasks[num_tasks].elapsed_time = 0;
    tasks[num_tasks].period = period;
    tasks[num_tasks].state = 0;
    tasks[num_tasks].flags = flags;
    tasks[num_tasks].tick = tick;
    num_tasks++;
}


void timer_ISR() {
    for (uint32_t i = 0; i < num_tasks; i++) {
        tasks[i].elapsed_time++;
        if (tasks[i].elapsed_time >= tasks[i].period) {
            tasks[i].elapsed_time = 0;
            tasks[i].state = (*tasks[i].tick)(tasks[i].state);
        }
    }
}

// Global IO buffers
unsigned char keypad_char;

// IO tick (update/flush buffers)
uint32_t tick_IO(uint32_t unused) {
    
    
    return 0;
}


uint32_t tick_KEYPAD(uint32_t unused) { // Unbuffered, since C is guaranteed exclusive
    PORTC = 0xEF;
    asm("nop");
    if (GET_BIT(PINC, 0) == 0) return keypad_char = '1';
    if (GET_BIT(PINC, 1) == 0) return keypad_char = '4';
    if (GET_BIT(PINC, 2) == 0) return keypad_char = '7';
    if (GET_BIT(PINC, 3) == 0) return keypad_char = '*';
    
    PORTC = 0xDF;
    asm("nop");
    if (GET_BIT(PINC, 0) == 0) return keypad_char = '2';
    if (GET_BIT(PINC, 1) == 0) return keypad_char = '5';
    if (GET_BIT(PINC, 2) == 0) return keypad_char = '8';
    if (GET_BIT(PINC, 3) == 0) return keypad_char = '0';
    
    PORTC = 0xBF;
    asm("nop");
    if (GET_BIT(PINC, 0) == 0) return keypad_char = '3';
    if (GET_BIT(PINC, 1) == 0) return keypad_char = '6';
    if (GET_BIT(PINC, 2) == 0) return keypad_char = '9';
    if (GET_BIT(PINC, 3) == 0) return keypad_char = '#';
    
    PORTC = 0x7F;
    asm("nop");
    if (GET_BIT(PINC, 0) == 0) return keypad_char = 'A';
    if (GET_BIT(PINC, 1) == 0) return keypad_char = 'B';
    if (GET_BIT(PINC, 2) == 0) return keypad_char = 'C';
    if (GET_BIT(PINC, 3) == 0) return keypad_char = 'D';
    return keypad_char = 0;
}

enum _state_LCD {S_LCD_NONE, S_LCD_RESET, S_LCD_IDLE, S_LCD_BUTTONDOWN, S_LCD_BUTTONWAIT};

const uint8_t MAX_OBSTACLES = 4;
uint8_t cacti[] = {0, 0, 0, 0};
    
uint8_t random8() {
    // This magic number seems to generate the most randomness.
    // Source: https://www.avrfreaks.net/forum/random-number-generation-0
    static uint8_t seed = 89;
    return seed = (seed * 109 + 89) % 251;
}

enum _state_OB {};
// Generate obstacles
uint32_t tick_OB(uint32_t state) {
    switch(state) {
        case
    }
}

uint32_t tick_LCD(uint32_t state) {

    switch(state) {
    }
    
    switch(state) {
    }
    return state;
}

int main(void) {
    // Keypad, unbuffered.
    INITIALISE_PORT(C, 0xF0);
    // LCD Ports
    INITIALISE_PORT(B, 0xFF);
    INITIALISE_PORT(D, 0xFF);
    LCD_init();
    LCD_ClearScreen();
    
    add_task(1, &tick_IO, TF_ENABLE | TF_REPEAT);
    add_task(1, &tick_KEYPAD, TF_ENABLE | TF_REPEAT);
    add_task(5, &tick_LCD, TF_ENABLE | TF_REPEAT);
    
    timer_init();
    timer_set(1);
    
    while (1) {
    }
}
