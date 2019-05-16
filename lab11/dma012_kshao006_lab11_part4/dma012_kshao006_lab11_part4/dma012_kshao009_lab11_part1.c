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
#include "common.h"
#include "lcd.h"



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

enum _state_LCD {S_LCD_BEGIN, S_LCD_SCROLL, S_LCD_END};


uint32_t tick_LCD(uint32_t state) {
    static const unsigned char dary[] = "CS120B is Legend... wait for it DARY!... who the fuck is DARY?";
    static const uint32_t dary_sz = sizeof(dary) - 1; // null character
    
    static uint32_t cnt = 0;
    static uint32_t pos = 0;
    
    switch(state) {
        case S_LCD_BEGIN:
        if (cnt >= 3) {
            cnt = 0;
            state = S_LCD_SCROLL;
        }
        break;
        
        case S_LCD_SCROLL:
        if (pos >= dary_sz - 16) {
            state = S_LCD_END;
        }
        break;
        
        case S_LCD_END:
        if (cnt >= 5) {
            cnt = 0;
            state = S_LCD_BEGIN;
        }
        break;
        
        default: 
        state = S_LCD_BEGIN;
        cnt = 0;
        break;
    }
    
    switch (state) {
        case S_LCD_BEGIN:
        pos = 0;
        if (cnt == 0) {
            for (uint8_t i = 0; i < 16; i++) {
                LCD_Cursor(i + 1);
                LCD_WriteData(dary[pos + i]);
            }
        }
        cnt++;
        break;
        
        case S_LCD_SCROLL:
        for (uint8_t i = 0; i < 16; i++) {
            LCD_Cursor(i + 1);
            LCD_WriteData(dary[pos + i]);      
        }
        pos++;
        break;
        
        case S_LCD_END:
        if (cnt == 0) {
            for (uint8_t i = 0; i < 16; i++) {
                LCD_Cursor(i + 1);
                LCD_WriteData(dary[pos + i]);
            }
        }
        
        cnt++;
        break;
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
    //add_task(1, *tick_KEYPAD, TF_ENABLE | TF_REPEAT);
    add_task(500, &tick_LCD, TF_ENABLE | TF_REPEAT);
    
    timer_init();
    timer_set(1);
    
    while (1) { 
    }
}

