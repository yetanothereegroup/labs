#ifndef LCD_H
#define LCD_H

void LCD_init();
void LCD_ClearScreen(void);
void LCD_WriteCommand (unsigned char Command);
void LCD_Cursor (unsigned char column);
void LCD_DisplayString(unsigned char column ,const unsigned char *string);
void delay_ms(int miliSec);

void LCD_WriteData(unsigned char Data);
void LCD_Cursor(unsigned char column);

#endif
