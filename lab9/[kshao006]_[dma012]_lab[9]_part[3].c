/*
 * [kshao006]_[dma012]_lab[9]_part[3].c
 *
 * Created: 5/1/2019 4:40:36 PM
 * Author : shaoken123
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"


volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned char button = 0x00;

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	OCR1A = 125;	
	TIMSK1 = 0x02; 
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}


ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TickSM(){
	while(!TimerFlag){}
	TimerFlag = 0;
}

#define C4  262
#define CS4 277
#define D4  294
#define DS4 311
#define E4  330
#define F4  349
#define FS4 370
#define G4  392
#define GS4 415
#define A4  440
#define AS4 466
#define B4  494
#define C5  523
#define CS5 554
#define D5  587
#define DS5 622
#define E5  659
#define F5  698
#define FS5 740
#define G5  784
#define GS5 831
#define A5  880
#define AS5 932
#define B5  988
#define C6  1047
#define CS6 1109
#define D6  1175
#define DS6 1245
#define E6  1319
#define F6  1397
#define FS6 1480
#define G6  1568
#define GS6 1661
#define A6  1760
#define AS6 1865
#define B6  1976
//the last of us:
double frequency[20] = {G5,E5,A5,FS5,G5,G5,E5,A5,FS5,B5,G5,E5,A5,FS5,G5,B5,AS5,G5,E5,E5};
unsigned short delay[20] = {100, 100, 100, 100, 800, 100, 100, 100, 100, 800, 100, 100, 100, 100, 800, 100, 100, 100, 100, 800};
unsigned short duration[20] = {500, 500, 500, 500, 1000, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 1000};

void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequen
	if (frequency != current_frequency) {
		if (!frequency) { TCCR0B &= 0x08; } //stops timer/counter
		else { TCCR0B |= 0x03; } // resumes/continues timer/counter
		if (frequency < 0.954) { OCR0A = 0xFFFF; }
		else if (frequency > 31250) { OCR0A = 0x0000; }
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
	//set_PWM(0);
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

enum Melody_SM{SM_start, s0, s1}state;
	
void tick(){
	unsigned char i;
	unsigned char cnt;
	switch(state){
		case SM_start:
		state = SM_start;
		i = 0;
		if(button){
			state = s0;
		}
		break;
		
		case s0:
		state = s1;
		break;
		
		case s1:
		if(button){
			state = s1;
		}
		else if(!button){
			state = SM_start;
		}
		break;
		
		default:
		state = SM_start;
		break;
	}
	switch(state){
		case SM_start:
		break;
		
		case s0:
		for(i = 0; i <= 19; ++i){
			set_PWM(frequency[i]);
			PWM_on();
			cnt = 0;
			while(cnt != (duration[i] / 100)){
				TickSM();
				++cnt;
			}
			PWM_off();
			cnt = 0;
			while(cnt != (delay[i] / 100)){
				TickSM();
				++cnt;
			}
		}
		break;
		
		case s1:
		break;
	}
}


int main(void)
{
    DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	TimerSet(100);
	TimerOn();
    while (1) 
    {
		button = ~PINA & 0x01;
		tick();
		TickSM();
    }
}

