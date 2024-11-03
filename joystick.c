#include <avr/io.h>
#include "ADC.h"
#include <avr/interrupt.h>
#include "io.c"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include "nokia5110.c"
#include "nokia5110.h"

#define LRPin 0
#define UDPin 1

signed short x_axis = 0x00;
signed short y_axis = 0x00;

unsigned char x = 42;
unsigned char y = 21;

signed short UDCenter = 512;//used in joystick calibration
signed short LRCenter = 512;//used in joystick calibration

unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}


unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}

unsigned char getLeft(signed short mid, signed short LR){
	return(LR > mid + 70);
}
unsigned char getRight(signed short mid, signed short LR){
	return (LR < mid - 70);
}
unsigned char getUp(signed short mid, signed short UD){
	return (UD > mid + 70);
}
unsigned char getDown(signed short mid, signed short UD){
	return (UD < mid - 70);
}



enum States_Joystick { start, x_read, y_read } state_joystick;

void joystick_tick() {
	
	switch(state_joystick) {
		case start:
		state_joystick = x_read;
		break;
		case x_read:
		state_joystick = y_read;
		break;
		case y_read:
		state_joystick = x_read;
		break;
		default: break;
	}
	switch(state_joystick) {
		case start:
		break;
		case x_read:
		x_axis = Read_ADC(LRPin);
		break;
		case y_read:
		y_axis = Read_ADC(UDPin);
		break;
		default: break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xF0; PORTD = 0x0F;
	
	
	unsigned char joystick_eTime = 5;
	unsigned char joystick_period = 5;
	unsigned char draw_eTime = 50;
	unsigned char draw_period = 50;
	unsigned char LCD_eTime = 50;
	unsigned char LCD_period = 50;
	const unsigned short timerPeriod = 10;

	state_joystick = start;
	x_axis = 0x0000;
	y_axis = 0x0000;
	TimerSet(timerPeriod);
	TimerOn();
	nokia_lcd_init();
	ADC_init();
	LCD_init();
	
	
	while (1)
	{
		
		if(joystick_eTime >= joystick_period) { // 5 ms period
			joystick_tick();	// Execute one tick of joystick_tick();
			joystick_eTime = 0;
		}
		if(draw_eTime >= draw_period) { // 50 ms period
			unsigned char up = getUp(UDCenter, y_axis);
			unsigned char down = getDown(UDCenter, y_axis);
			unsigned char left = getLeft(LRCenter, x_axis);
			unsigned char right = getRight(LRCenter, x_axis);
			
			if(up) {
				if(y < 42) {
					y--;
				}
			}
			if(down) {
				if(y > 1) {
					y++;
				}
			}              
			if(left) {
				if(x < 84) {
					x++;
				}
			}
			if(right) {
				if(x > 1) {
					x--;
				}
			}
			nokia_lcd_set_pixel(x, y, 1);
			nokia_lcd_render();
			draw_eTime = 0;
		}
		
		if(LCD_eTime >= LCD_period)	{
			char x_value[3]; /* A buffer to hold the ascii string */
			char y_value[3]; /* A buffer to hold the ascii string */
			
			for(unsigned char i = 0; i < 3; i++) {
				x_value[i] = ' ';
				y_value[i] = ' ';
			}			
			sprintf(&x_value[0], "%d", x); //hex to string
			sprintf(&y_value[0], "%d", y); //hex to string
				
			LCD_WriteData(1);
			LCD_Cursor(2);
			LCD_WriteData(2);
			LCD_Cursor(17);
			LCD_WriteData(3);
			LCD_Cursor(18);
			LCD_WriteData(4);
			
			unsigned char c1 = 3;
			for(unsigned char i = 0; i < 2; i++) {
				LCD_Cursor(c1);
				LCD_WriteData(x_value[i]);
				c1++;
			}
			unsigned char c2 = 19;
			for(unsigned char i = 0; i < 2; i++) {
				LCD_Cursor(c2);
				LCD_WriteData(y_value[i]);
				c2++;
			}
			
			LCD_eTime = 0;
		}
		
		//button_tick();

		while (!TimerFlag);
		TimerFlag = 0;
		
		joystick_eTime += timerPeriod;
		draw_eTime += timerPeriod;
		LCD_eTime += timerPeriod;
		
	}
			}