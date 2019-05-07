/*	Partner 1 Name & E-mail: Ken Shao, kshao006@ucr.edu, 862048975
 *	Partner 2 Name & E-mail: Daneil Ma, dma012@ucr.edu
 *	Lab Section: 023
 *	Assignment: Lab 6  Exercise 2 
 *	Exercise Description: Challenge
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
volatile unsigned char button = 0;

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
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
	while(!TimerFlag){
		if((PINA & 0x01) == 0x00){
			button = 0x01;
		}
	}
	TimerFlag = 0;
}

enum ex2_led { ex2_SMStart, ex2_s0, ex2_s1, ex2_s2, ex2_s3, ex2_s4, waitButton, waitButton_2} state;
enum statePrev {start, s0, s1, s3} statePrev ;

void exercise2_led(){
	switch(state) { // Transitions
		case ex2_SMStart:
		//TimerFlag = 0;
		state = ex2_s0;
		if(button == 0x01){
			statePrev = start;
			button = 0;
			state = waitButton_2;
		}
		break;

				
		case ex2_s0:
		//TimerFlag = 0;
		state = ex2_s1;
		if(button == 0x01){
			statePrev = s0;
			button = 0;
			state = waitButton_2;
		}
		break;
		
		case ex2_s1:
		//TimerFlag = 0;
		state = ex2_s3;
		if(button == 0x01){
			statePrev = s1;
			button = 0;
			state = waitButton_2;
		}
		break;
		
		case ex2_s2:
		//TimerFlag = 0;
		state = ex2_s2;
		if(button == 0x01){
			state = waitButton;
		}
		break;
		
		case ex2_s3:
		//TimerFlag = 0;
		state = ex2_SMStart;
		if(button == 0x01){
			statePrev = s3;
			button = 0;
			state = waitButton_2;
		}
		break;
		
		case waitButton:
		//TimerFlag = 0;
		if((PINA & 0x01) == 0x00){
			state = waitButton;
		}
		else{
			button = 0;
			state = ex2_s4;
			TimerFlag = 1;
		}
		break;
		
		case waitButton_2:
		if((PINA & 0x01) == 0x00){
			state = waitButton_2;
		}
		else{
			button = 0;
			state = ex2_s2;
			TimerFlag = 1;
		}
		break;
		
		case ex2_s4:
			if(statePrev == start){
				state = ex2_SMStart;
			}
			if(statePrev == s0){
				state = ex2_s0;
			}
			if(statePrev == s1){
				state = ex2_s1;
			}
			if(statePrev == s3){
				state = ex2_s3;
			}
			TimerFlag = 1;
		break;
		
		default:
		state = ex2_SMStart;
		break;
	}
	
	switch(state) { // Actions
		case ex2_SMStart:
		PORTB = 0x01;
		break;
		
		case ex2_s0:
		PORTB = 0x02;
		break;
		
		case ex2_s1:
		PORTB = 0x04;
		break;
		
		case ex2_s3:
		PORTB = 0x02;
		break;
		
		default:
		break;
	}
}

void main()
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	TimerSet(300);
	TimerOn();
	while(1) {
		exercise2_led();
		TickSM();
	}
}

