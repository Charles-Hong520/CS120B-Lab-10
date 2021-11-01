/*	Author: Charles Hong
 *  Partner(s) Name:  
 *	Lab Section: 022
 *	Assignment: Lab #10  Exercise #1
 *	Exercise Description: Three syncSM with B3-B0
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demo Link: 
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; //TimerISR sets it to 1, programmer sets it to 0
unsigned long _avr_timer_M = 1; //start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; //current internal count of 1ms ticks

void TimerOn() {
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1  = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0) {
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

enum ThreeLEDsSM {TLstart, zero, one, two} TLstate;
enum BlinkingLEDSM {BLstart, on, off} BLstate;
unsigned char threeLEDs, blinkingLED;
void ThreeLEDsTick() {
    switch(TLstate) {
        case TLstart:
        TLstate = zero;
        threeLEDs = 0;
        break;
        case zero:
        TLstate = one;
        break;
        case one:
        TLstate = two;
        break;
        case two:
        TLstate = zero;
        break;
    }
    switch(TLstate) {
        case TLstart:
        break;
        case zero:
        threeLEDs = 0x01;
        break;
        case one:
        threeLEDs = 0x02;
        break;
        case two:
        threeLEDs = 0x04;
        break;
    }
}

void BlinkingLEDTick() {
    switch(BLstate) {
        case BLstart:
        BLstate = off;
        blinkingLED = 0;
        break;
        case off:
        BLstate = on;
        break;
        case on:
        BLstate = off;
        break;
    }
    switch(BLstate) {
        case BLstart:
        break;
        case off:
        blinkingLED = 0x00;
        break;
        case on:
        blinkingLED = 0xFF;
        break;
    }
}

void CombineLEDsTick() {
    ThreeLEDsTick();
    BlinkingLEDTick();
    PORTB = (blinkingLED&0x08)+threeLEDs;
}

int main(void) {
    DDRB = 0x0F; PORTB = 0x00;
    TimerSet(1000); TimerOn();
    while (1) {
        TLstate = TLstart;
        BLstate = BLstart;
        CombineLEDsTick();
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
