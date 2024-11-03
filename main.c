/*
 * FinalProjectCode.c
 *
 * Created: 3/12/2019 7:02:55 PM
 * Author : mehra
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "io.c"
#include "ADC.h"

/*---Joystick1-----*/
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

unsigned char getLeft(signed short mid, signed short LR){
	return(LR < mid - 15);
}
unsigned char getRight(signed short mid, signed short LR){
	return (LR > mid + 15);
}
unsigned char getUp(signed short mid, signed short UD){
	return (UD > mid + 70);
}
unsigned char getDown(signed short mid, signed short UD){
	return (UD < mid - 70);
}

/*---------JOYSTICK END---------------------*/
/*---------BALL()---------------------------*/
unsigned char jump = 0x00;
unsigned char change = 0x00;
unsigned char middlecheck = 0x00;
unsigned char initctr = 0x00;
unsigned char terminate = 0x00;
enum ball_states { start, clockwise, ctrclockwise, clockwiseMiss, ctrclockwiseMiss, endgame} ball_state;
void ball() {
	unsigned char D = (~PORTD) & 0xE7;
	unsigned char cD = ~PORTD;
		
	switch(ball_state){
		case start:
		if (getUp(UDCenter, y_axis)) {
			initctr = 0;
			ball_state = clockwise;
			PORTB = 0x01; PORTD = 0xFE;
		}
		else if (initctr % 2 == 1) {
			ball_state = start;
			initctr++;
			PORTB = 0x01; PORTD = 0xE7;
		}
		else if (initctr % 2 == 0) {
			ball_state = start;
			initctr++;
			PORTB = 0x01; PORTD = 0xE6;
		}
		break;
		case clockwise:
		if (getRight(LRCenter, x_axis) && (PORTD == 0xC7) && (PORTB == 0x01)) { //ball is clockwise towards p1  //~D == PORTD == 0xDF
			change = 0x01;
			ball_state = ctrclockwise;
		}
		else if (getRight(LRCenter, x_axis) && !((((PORTD == 0xC7) || (PORTD == 0xA7) || (PORTD == 0x67) || (PORTD == 0x7F)) && ((PORTB == 0x01) || (PORTB == 0x02))))) { //0xDF, 0xBF, 0x7F
			ball_state = clockwiseMiss;
		}
		else if (!(getRight(LRCenter, x_axis)) && !(getUp(UDCenter, y_axis)) && (PORTD == 0xE7) && (PORTB == 0x01))  {   //PORTD = EF
			ball_state = endgame;
		}
		else {
		    change = 0x00;
			ball_state = clockwise;
		}
		break;
		case ctrclockwise:
			if (getLeft(LRCenter, x_axis) && ((PORTD == 0xE3) && (PORTB == 0x01))) {
				change = 0x01;
				ball_state = clockwise;
			}
			else if (getLeft(LRCenter, x_axis) && !((((PORTD == 0xE3) || (PORTD == 0xE5) || (PORTD == 0xE6) || (PORTD == 0xFE)) && ((PORTB == 0x01) || (PORTB == 0x02))))) {
				ball_state = ctrclockwiseMiss;
			}
			else if (!(getLeft(LRCenter, x_axis)) && !(getUp(UDCenter, y_axis)) && (PORTD == 0xE7) && (PORTB == 0x01)) {
				ball_state = endgame;
			}
			else {
				change = 0x00;
				ball_state = ctrclockwise;
			}
			break;
		case clockwiseMiss:
			if (!(getRight(LRCenter, x_axis))) {
				ball_state = clockwise;
			}
			else if (!(getUp(UDCenter, y_axis)) && ((PORTD == 0xE7) &&  (PORTB == 0x01))) {
				ball_state = endgame;
			}
			else {
				ball_state = clockwiseMiss;
			}
			break;
		case ctrclockwiseMiss:
			if (!(getLeft(LRCenter, x_axis))) {
				ball_state = ctrclockwise;
			}
			else if (!(getUp(UDCenter, y_axis)) && ((PORTD == 0xE7) &&  (PORTB == 0x01))) {
				ball_state = endgame;
			}
			else {
				ball_state = ctrclockwiseMiss;
			}
			break;
		case endgame:
			if ((PINA & 0x10) == 0x00) { //reset
				ball_state = start;
			}
			break;
		default: ball_state = start; break;
	}
	switch(ball_state) {
		case start:
			terminate = 0x00;
			break;
		case clockwise:
			if (!(PORTD & 0x01) && (PORTB < 0x80)) {
				PORTB = PORTB << 1;
				PORTD = PORTD & 0xE7;
			}
			else if ((PORTD > 0x7F) && (PORTB & 0x80)) {
				if (PORTD == 0xFB) {
					PORTD = 0xF7;
				}
				else if (PORTD == 0xF7) {
					PORTD = 0xEF;
				}
				else if (PORTD == 0xEF) {
					PORTD = 0xDF;
				}
				else {
					PORTD = ~(D << 1);
				}
				
			}
			else if (!(PORTD & 0x80) && (PORTB > 0x01)) {
				PORTB = PORTB >> 1;
				PORTD = PORTD & 0xE7;
			}
			else if ((PORTD < 0xFE) && ((PORTB & 0x01) || (jump))) {
				if (PORTD == 0xDF) {
					PORTD = 0xE7;
				}
				else if (PORTD == 0xE7) {
					middlecheck++;
					if (getUp(UDCenter, y_axis)) {
						jump = 0x01;
						PORTB = PORTB | 0x02;
					}
					if (middlecheck == 0x02) {
						PORTD = 0xE3;
						middlecheck = 0x00;
						jump = 0x00;
						PORTB = 0x01;
					}
					else {
						PORTD = 0xE7;
					}
				}
				else {
					PORTD = ~(D >> 1) & (0xE7);
				}
			}
			break;
		case ctrclockwise:
			if ((PORTD > 0x7F) && ((PORTB == 0x01) || (jump))) {
				if (PORTD == 0xE3) {
					PORTD = 0xE7;
				}
				else if (PORTD == 0xE7) {
					/*-------JUMP ANIMATION------------*/
					middlecheck++;
					if (getUp(UDCenter, y_axis)) {
						jump = 0x01;
						PORTB = PORTB | 0x02;
					}
					if (middlecheck == 0x02) {
						PORTD = 0xC7;
						middlecheck = 0;
						jump = 0x00;
						PORTB = 0x01;
					}
					else {
						PORTD = 0xE7;
					}
				}
				else {
					PORTD = ~(D << 1) & (0xE7);
				}
			}
			else if (!(PORTD & 0x80) && (PORTB < 0x80)) {
				PORTB = PORTB << 1;
				PORTD = PORTD & 0xE7;
			}
			else if ((PORTD < 0xFE) && (PORTB & 0x80)) {
				if (PORTD == 0xDF) {
					PORTD = 0xEF;
				}
				else if (PORTD == 0xEF) {
					PORTD = 0xF7;
				}
				else if (PORTD == 0xF7) {
					PORTD = 0xFB;
				}
				else {
					PORTD = ~(D >> 1);
				}
				  //~(D >>1 )
				
			}
			else if (!(PORTD & 0x01) && (PORTB > 0x01)) {
				PORTB = PORTB >> 1;
				PORTD = PORTD & 0xE7;
			}
			break;
		case clockwiseMiss:
			if (!(PORTD & 0x01) && (PORTB < 0x80)) {
				PORTB = PORTB << 1;
				PORTD = PORTD & 0xE7;
			}
			else if ((PORTD > 0x7F) && (PORTB & 0x80)) {
				if (PORTD == 0xFB) {
					PORTD = 0xF7;
				}
				else if (PORTD == 0xF7) {
					PORTD = 0xEF;
				}
				else if (PORTD == 0xEF) {
					PORTD = 0xDF;
				}
				else {
					PORTD = ~(D << 1);
				}
				
			}
			else if (!(PORTD & 0x80) && (PORTB > 0x01)) {
				PORTB = PORTB >> 1;
				PORTD = PORTD & 0xE7;
			}
			else if ((PORTD < 0xFE) && (PORTB & 0x01)) {
				if (PORTD == 0xDF) {
					PORTD = 0xE7;
				}
				else if (PORTD == 0xE7) {
					middlecheck++;
					if (middlecheck == 0x02) {
						PORTD = 0xE3;
						middlecheck = 0x00;
					}
					else {
						PORTD = 0xE7;
					}
				}
				else {
					PORTD = ~(D >> 1) & (0xE7);
				}
			}
			break;
		case ctrclockwiseMiss:
			if ((PORTD > 0x7F) && (PORTB == 0x01)) {
				if (PORTD == 0xE3) {
					PORTD = 0xE7;
				}
				else if (PORTD == 0xE7) {
					middlecheck++;
					if (middlecheck == 0x02) {
						PORTD = 0xC7;
						middlecheck = 0;
					}
					else {
						PORTD = 0xE7;
					}
				}
				PORTD = ~(D << 1) & (0xE7);
			}
			else if (!(PORTD & 0x80) && (PORTB < 0x80)) {
				PORTB = PORTB << 1;
				PORTD = PORTD & 0xE7;
			}
			else if ((PORTD < 0xFE) && (PORTB & 0x80)) {
				if (PORTD == 0xDF) {
					PORTD = 0xEF;
				}
				else if (PORTD == 0xEF) {
					PORTD = 0xF7;
				}
				else if (PORTD == 0xF7) {
					PORTD = 0xFB;
				}
				else {
					PORTD = ~(D >> 1);
				}
				//~(D >>1 )
				
			}
			else if (!(PORTD & 0x01) && (PORTB > 0x01)) {
				PORTB = PORTB >> 1;
				PORTD = PORTD & 0xE7;
			}
			break;
		case endgame:
			PORTB = 0xFF;
			PORTD = 0x00;
			terminate = 0x01;
			break;
		default:
			break;
	}
}

enum States_Joystick { Start, x_read, y_read } state_joystick;

void joystick_tick() {
	
	switch(state_joystick) {
		case Start:
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
		case Start:
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
	
	DDRB = 0xFF;
	PORTB = 0x01;
	DDRD = 0xFF;
	PORTD = 0xFE;
	DDRC = 0x00;
	const unsigned long timerPeriod = 10;
	
	unsigned long ball_elapsedTime = 200;
	unsigned long ballSpeed = 200;
	//unsigned char ballctr = 0x00;
	unsigned char joystick_eTime = 5;
	unsigned char joystick_period = 5;
	
	state_joystick = start;
	x_axis = 0x0000;
	y_axis = 0x0000;
	TimerSet(timerPeriod);
	TimerOn();
	ADC_init();
	
    /* Replace with your application code */
    while (1) 
    {
		/*-----Displaying Player-----------*/
		
		if(joystick_eTime >= joystick_period) { // 5 ms period
			joystick_tick();	// Execute one tick of joystick_tick();
			joystick_eTime = 0;
			
		}
		if (ball_elapsedTime >= ballSpeed) {
			ball();
			ball_elapsedTime = 0;
			if (change) {
				change = 0x00;
				if (ballSpeed <= 100) {
					ballSpeed = ballSpeed - 10;
				}
				else if (ballSpeed <= 200) {
					ballSpeed = ballSpeed - 20;
				}
				else {
					ballSpeed = ballSpeed - 50;
				}
			}
			if (terminate) {
				ballSpeed = 200; //starting speed of choice
			}
		}
		
		while(!TimerFlag) {}
		TimerFlag = 0;
		joystick_eTime += timerPeriod;
		ball_elapsedTime += timerPeriod;
    }
}

