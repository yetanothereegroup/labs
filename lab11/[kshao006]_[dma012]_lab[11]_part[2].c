/*	Partner 1 Name & E-mail: Ken Shao, kshao006@ucr.edu, 862048975
 *	Partner 2 Name & E-mail: Daneil Ma, dma012@ucr.edu
 *	Lab Section: 023
 *	Assignment: Lab 11  Exercise 2
 *	Exercise Description: using structs
 *
 *	Author: Ken Shao
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"


volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned char button = 0x00;

unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

unsigned char GetBit(unsigned char port, unsigned char number)
{
	return ( port & (0x01 << number) );
}


typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
}task;

const unsigned char tasksNum = 2;
task tasks[2];
const unsigned long tasksPeriodGCD = 1;
const unsigned long periodKey = 1;
const unsigned long periodShow = 150;
unsigned char button_inc;
unsigned char button_dec;


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

/*void TimerISR() {
	TimerFlag = 1;
}*/

void TimerISR() {
	unsigned char i;
	button = ~PINA & 0x04;
	button_dec = ~PINA & 0x02;
	button_inc = ~PINA & 0x01;
	for (i = 0; i < tasksNum; ++i) {
		if (tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

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

void set_PWM(double frequency) {
	static double current_frequency;
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
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	//set_PWM(0);
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

unsigned char currKey = 0;
enum keypad {key_s0, key_s1, key_s2, key_s3, key_s4};
int TickFct_getkeypad(int state)
{
	unsigned char counter = 0;
	switch(state) { // Transitions
		case key_s0:
		state = key_s1;
		break;
		
		case key_s1:
		state = key_s2;
		break;
		
		case key_s2:
		state = key_s3;
		break;
		
		case key_s3:
		state = key_s4;
		break;
		
		case key_s4:
		state = key_s0;
		break;
	}
	
	switch(state) { // Actions
		case key_s0:
		PORTC = 0xEF;
		asm("nop");
		if (GetBit(PINC,0)==0) { currKey = '1';}
		if (GetBit(PINC,1)==0) { currKey = '4';}
		if (GetBit(PINC,2)==0) { currKey = '7';}
		if (GetBit(PINC,3)==0) { currKey = '*';}
		//if (PINC == 0x0F) { currKey = '0';}
		//PORTC = 0x00;
		break;
		
		case key_s1:
		PORTC = 0xDF;
		asm("nop");
		if (GetBit(PINC,0)==0) { currKey = '2';}
		if (GetBit(PINC,1)==0) { currKey = '5';}
		if (GetBit(PINC,2)==0) { currKey = '8';}
		if (GetBit(PINC,3)==0) { currKey = '0';}
		//if (PINC == 0x0F) { currKey = '0';}
		//PORTC = 0x00;
		break;
		
		case key_s2:
		PORTC = 0xBF;
		asm("nop");
		if (GetBit(PINC,0)==0) { currKey = '3';}
		if (GetBit(PINC,1)==0) { currKey = '6';}
		if (GetBit(PINC,2)==0) { currKey = '9';}
		if (GetBit(PINC,3)==0) { currKey = '#';}
		//if (PINC == 0x0F) { currKey = '0';}
		//PORTC = 0x00;
		break;
		
		case key_s3:
		PORTC = 0x7F;
		asm("nop");
		if (GetBit(PINC,0)==0) { currKey = 'A';}
		if (GetBit(PINC,1)==0) { currKey = 'B';}
		if (GetBit(PINC,2)==0) { currKey = 'C';}
		if (GetBit(PINC,3)==0) { currKey = 'D';}
		//if (PINC == 0x0F) { currKey = '0';}
		//PORTC = 0x00;
		break;
		
		case key_s4:
		break;
	}
	return state;
}

signed char show_counter = 32;

enum show {show_s0, show_s1, show_s2, show_s3, show_s4};
int TickFct_show(int state)
{
	switch(state) { // Transitions
		case show_s0:
		state = show_s0;
		if(button){
			state = show_s3;
		}
		break;
		
		case show_s3:
		if(button){
			state = show_s3;
		}
		if(!button){
			state = show_s1;
		}
		break;
		
		case show_s1:
		state = show_s2;
		if(button){
			state = show_s4;
		}
		break;
		
		case show_s2:
		state = show_s1;
		if(button){
			state = show_s4;
		}
		break;
		
		case show_s4:
		if(button){
			state = show_s4;
		}
		if(!button){
			state = show_s0;
		}
		break;
	
		default:
		state = show_s0;
		break;
	}
	
	switch (state) {
		case show_s0:
		LCD_ClearScreen();
		break;
		
		case show_s2:
		--show_counter;
		break;
		
		case show_s1:
		if(show_counter == -32){
			show_counter = 32;
		}
		LCD_ClearScreen();
		LCD_DisplayString(show_counter, "CS120B is Legend... wait for it DARY!");
		if(show_counter >= 21){
			LCD_Cursor(1);
			LCD_WriteData(' ');
			LCD_Cursor(0);
		}
		if(show_counter <= -21){
			LCD_Cursor(17);
			LCD_WriteData(' ');
			LCD_Cursor(0);
		}
		break;
	}
	return state;
}

int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	
	LCD_init();
	
	unsigned char i = 0;
	tasks[i].state = key_s0;
	tasks[i].period = periodKey;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_getkeypad;
	++i;
	tasks[i].state = show_s0;
	tasks[i].period = periodShow;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_show;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
    while (1){
	}
	return 0;
}

