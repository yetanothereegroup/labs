/*
 * io.h
 *
 * Created: 4/24/2019 20:35:38
 *  Author: danie
 */ 


#ifndef IO_H_
#define IO_H_


void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column ,const char *string);
void delay_ms(int miliSec);
void LCD_WriteData(unsigned char Data);


#endif /* IO_H_ */