/*	Partner 1 Name & E-mail: Ken Shao, kshao006@ucr.edu, 862048975
 *	Partner 2 Name & E-mail: Daneil Ma, dma012@ucr.edu
 *	Lab Section: 023
 *	Assignment: Lab 11  Exercise 5
 *	Exercise Description: dodge game with pause feature.
 *	LCD is PORTD, control is B6 and B7
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

const unsigned char tasksNum = 4;
task tasks[4];
const unsigned long tasksPeriodGCD = 5;
const unsigned long periodPlayer = 5;
const unsigned long periodObstacle = 350;
const unsigned long periodCheck = 10;
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

enum obstacle_bot {bob_s0, bob_s1, bob_over, bob_pause, bob_waitstart};
enum obstacle {tob_s0, tob_s1, tob_s2, tob_s3, tob_over, tob_pause, tob_waitstart};

unsigned char currKey = 0;
unsigned char prevCursor = 240;
unsigned char pause_statue = 0xFF;
enum player_button {player_s0, player_s1, player_s2, player_s3, player_s4, player_s2_wait, player_s3_wait, player_s4_wait, player_over, player_waitstart};
int TickFct_Player(int state)
{
	unsigned char counter = 0;
	switch(state) { // Transitions
		case player_waitstart:
		state = player_waitstart;
		if(button){
			state = player_s0;
			LCD_ClearScreen();
			tasks[0].period = periodPlayer;
		}
		LCD_DisplayString(1, "Press Start!");
		break;
		
		case player_s0:
		state = player_s1;
		LCD_ClearScreen();
		LCD_Cursor(2);
		break;
		
		case player_s1:
		state = player_s1;
		if(button_inc){
			state = player_s2;
		}
		if(button_dec){
			state = player_s3;
		}
		if(button){
			state = player_s4;
		}
		break;
		
		case player_s2:
		state = player_s2_wait;
		break;
		
		case player_s2_wait:
		if(button_inc){
			state = player_s2_wait;
		}
		else{
			state = player_s1;
		}
		break;
		
		case player_s3:
		state = player_s3_wait;
		break;
		
		case player_s3_wait:
		if(button_inc){
			state = player_s3_wait;
		}
		else{
			state = player_s1;
		}
		break;
		
		case player_s4:
		state = player_s4_wait;
		break;
		
		case player_s4_wait:
		if(button){
			state = player_s4_wait;
		}
		else{
			state = player_s1;
		}
		break;
		
		case player_over:
		state = player_over;
		break;
	}
	
	switch(state) { // Actions
		case player_s2:
		LCD_Cursor(2);
		prevCursor = 2;
		break;
		
		case player_s3:
		LCD_Cursor(18);
		prevCursor = 18;
		break;
		
		case player_s4:
		pause_statue = !pause_statue;
		if(pause_statue){
			tasks[1].state = tob_pause;
			tasks[2].state = bob_pause;
		}
		else if(!pause_statue){
			tasks[1].state = tob_s1;
			tasks[2].state = bob_s1;
		}
		break;
	}
	return state;
}

unsigned char tob_cnt = 0;
unsigned char tob_cnt_2 = 0;
unsigned char tob_pos[3] = {0,0,0};

//enum obstacle {tob_s0, tob_s1, tob_s2, tob_s3, tob_over, tob_pause};
int TickFct_topobstacles(int state)
{
	switch(state) { // Transitions
		case tob_waitstart:
		state = tob_waitstart;
		if(button){
			state = tob_s0;
		}
		break;
		
		case tob_s0:
		tob_pos[0] = 16;
		tob_pos[1] = 16;
		tob_pos[2] = 0;
		state = tob_s0;
		++tob_cnt;
		if(tob_cnt == 6){
			state = tob_s1;
			tob_cnt = 0;
		}
		break;
		
		case tob_s1:
		state = tob_s1;
		++tob_cnt;
		++tob_cnt_2;
		if(tob_cnt >= rand()%16){
			tob_cnt = 0;
			if(tob_pos[1] == 0){
				tob_pos[1] = 16;
			}
		}
		if(tob_cnt_2 >= rand()%45){
			tob_cnt_2 = 0;
			if(tob_pos[2] == 0){
				tob_pos[2] = 16;
			}
		}
		break;
		
		case tob_over:
		state = tob_over;
		break;
		
		case tob_pause:
		state = tob_pause;
		break;
		
		default:
		state = tob_s0;
		break;
	}
	
	switch (state) {
		
		case tob_s1:
		LCD_ClearScreen();
		LCD_Cursor(tob_pos[0]);
		//LCD_WriteData('#'); 
		LCD_Cursor(prevCursor);
		if(tob_pos[0] > 0){
			--tob_pos[0];
		}
		LCD_Cursor(tob_pos[1]);
		LCD_WriteData('#');
		LCD_Cursor(prevCursor);
		if(tob_pos[1] > 0){
			--tob_pos[1];
		}
		LCD_Cursor(tob_pos[2]);
		LCD_WriteData('#');
		LCD_Cursor(prevCursor);
		if(tob_pos[2] > 0){
			--tob_pos[2];
		}
		break;
	}
	return state;
}

unsigned char bob_cnt = 0;
unsigned char bob_cnt_2 = 0;
unsigned char bob_pos[3] = {0,0,0};

//enum obstacle_bot {bob_s0, bob_s1, bob_over, bob_pause, bob_pause_wait};
int TickFct_botobstacles(int state)
{
	switch(state) { // Transitions
		case bob_waitstart:
		state = bob_waitstart;
		if(button){
			state = bob_s0;
		}
		break;
		
		case bob_s0:
		bob_pos[0] = 32;
		bob_pos[1] = 32;
		bob_pos[2] = 32;
		state = bob_s0;
		++bob_cnt;
		if(bob_cnt == 18){
			state = bob_s1;
			bob_cnt = 0;
		}
		break;
		
		case bob_s1:
		state = bob_s1;
		++bob_cnt;
		++bob_cnt_2;
		if(bob_cnt >= 20){
			bob_cnt = 0;
			bob_pos[1] = 32;
		}
		if(bob_cnt_2 >= 31){
			bob_cnt_2 = 0;
			bob_pos[2] = 32;
		}
		break;
		
		case bob_over:
		state = bob_over;
		break;
		
		case bob_pause:
		state = bob_pause;
		break;
		
		default:
		state = bob_s0;
		break;
	}
	
	switch (state) {
		
		case bob_s1:
		//LCD_ClearScreen();
		LCD_Cursor(bob_pos[0]);
		//LCD_WriteData('#');
		LCD_Cursor(prevCursor);
		if(bob_pos[0] > 16){
			--bob_pos[0];
		}
		if(bob_pos[1] == 16){
			bob_pos[1] = 40;
		}
		LCD_Cursor(bob_pos[1]);
		LCD_WriteData('#');
		LCD_Cursor(prevCursor);
		if(bob_pos[1] > 16){
			--bob_pos[1];
		}
		if(bob_pos[2] == 16){
			bob_pos[2] = 55;
		}
		LCD_Cursor(bob_pos[2]);
		LCD_WriteData('#');
		LCD_Cursor(prevCursor);
		if(bob_pos[2] > 16){
			--bob_pos[2];
		}
		break;
		
		case bob_pause:
		break;
	}
	return state;
}

enum check_game {check_s0, check_s1};
int TickFct_Check(int state)
{
	switch(state) { // Transitions
		case check_s0:
		state = check_s0;
		if(prevCursor - 1 == tob_pos[1] || prevCursor - 1 == tob_pos[2] || prevCursor - 1 == bob_pos[1] || prevCursor - 1 == bob_pos[2]){
			state = check_s1;
		}
		break;
		
		case check_s1:
		state = check_s1;
		break;
		
	}
	
	switch(state) { // Actions
		case check_s1:
		tob_pos[1] = 0;
		tob_pos[2] = 0;
		bob_pos[1] = 0;
		bob_pos[2] = 0;
		tasks[0].state = player_over;
		tasks[1].state = tob_over;
		tasks[2].state = bob_over;
		tasks[3].period = 1000;
		LCD_DisplayString(1, "GAME OVER !!!");
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
	tasks[i].state = player_waitstart;
	tasks[i].period = 400;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Player;
	++i;
	tasks[i].state = tob_waitstart;
	tasks[i].period = periodObstacle;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_topobstacles;
	++i;
	tasks[i].state = bob_waitstart;
	tasks[i].period = periodObstacle;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_botobstacles;
	++i;
	tasks[i].state = check_s0;
	tasks[i].period = periodCheck;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Check;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
    while (1){
	}
	return 0;
}
