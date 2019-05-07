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
#include "common.h"

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }

int main(void) {
    
    while (1) {
    }
}

