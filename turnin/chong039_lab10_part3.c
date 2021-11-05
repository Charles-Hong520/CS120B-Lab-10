/*	Author: Charles Hong
 *  Partner(s) Name:  
 *	Lab Section: 022
 *	Assignment: Lab #10  Exercise #3
 *	Exercise Description: Three syncSM with B3-B0 and speaker
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *  
 *  Demo Link: https://drive.google.com/file/d/1yBprb03gRer0xF1g8Ns88YYrzzubQgb3/view?usp=sharing
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
enum PWMSM {PWMstart, hi, lo} PWMstate;
unsigned char threeLEDs, blinkingLED;
unsigned long BL_elapsedTime=0, TL_elapsedTime=0;
unsigned char i,speaker, H=2, L=2;
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
        blinkingLED = 0x01;
        break;
    }
}

void PWMTick() {
    switch(PWMstate) {
        case PWMstart:
        PWMstate = hi;
        i = speaker = 0;
        break;
        case hi:
        if(i>=H) {
            PWMstate = lo;
            speaker = 0;
            i=0;
        }
        break;
        case lo:
        if(i>=L) {
            PWMstate = hi;
            if((PINA&0x04)==0x04) {
                speaker = 1;
            } else {
                speaker = 0;
            }
            i=0;
        }
        break;
    }
    switch(PWMstate) {
        case PWMstart: break;
        case hi:
        if((PINA&0x04)==0x04) {
            speaker = 1;
        } else {
            speaker = 0;
        }
        i++;
        break;
        case lo:
        i++;
        speaker = 0;
        break;
    }
}

void CombineLEDsTick() {
    if(TL_elapsedTime>=300) {
        ThreeLEDsTick();
        TL_elapsedTime = 0;
    }
    if(BL_elapsedTime>=1000) {
        BlinkingLEDTick();
        BL_elapsedTime = 0;
    }
    PWMTick();
    PORTB = (speaker<<4)+(blinkingLED<<3)+threeLEDs;
}

int main(void) {
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0x1F; PORTB = 0x00;
    TLstate = TLstart;
    BLstate = BLstart;
    TimerSet(1);
    TimerOn();
    while (1) {
        CombineLEDsTick();
        while(!TimerFlag);
        TimerFlag = 0;
        TL_elapsedTime++;
        BL_elapsedTime++;
    }
    return 1;
}
