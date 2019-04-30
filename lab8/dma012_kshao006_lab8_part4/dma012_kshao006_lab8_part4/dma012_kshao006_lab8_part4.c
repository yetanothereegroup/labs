/*
 * Daniel Ma <dma012@ucr.edu>
 * Ken Shao <kshao006@ucr.edu>
 *
 * Lab section: 023
 * Assignment: dma012_kshao006_lab8_part4
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 4/29/2019 21:01:16
 *
 * 
 */ 

#include <avr/io.h>

#define PHOTO_MAX 0x00b0  // Max when exposed to light.

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }

void ADC_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
    // ADEN: setting this bit enables analog-to-digital conversion.
    // ADSC: setting this bit starts the first conversion.
    // ADATE: setting this bit enables auto-triggering. Since we are
    //        in Free Running Mode, a new conversion will trigger whenever
    //        the previous conversion completes.
}


int main(void) {
    ADC_init();
    
    uint16_t buf_adc;
    uint8_t buf_b;
    
    const uint16_t step = (PHOTO_MAX / 8); // BUG: beware of precision loss
    
    INITIALISE_PORT(B, 0xFF);
    INITIALISE_PORT(D, 0xFF);
    while (1) {
        buf_adc = ADC;
        buf_b = 0;
        int i = 0;
        for (i = 0; i < 8; i++) {
            if (buf_adc > i * step) {
                SET_BIT(buf_b, i, 1);
            }
        }
        
        
        PORTB = buf_b;
        
    }
}

