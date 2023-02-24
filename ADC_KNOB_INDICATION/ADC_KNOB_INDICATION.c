#device PIC16F877A
#include <stdio.h> 
#include <16F877A.h>
#fuses HS,NOPROTECT,NOBROWNOUT,NOWDT,NOPUT,NOCPD,NOLVP,NOWRT
#use delay(clock=20000000)
#byte TRISD = 0x88 // IO Mode
#byte PORTD = 0x08 // IO Port

#byte TMR0 = 0x01
#byte OPTION_REG = 0x81
	#bit T0CS = OPTION_REG.5
	#bit PSA = OPTION_REG.3
	#bit PS2 = OPTION_REG.2
	#bit PS1 = OPTION_REG.1
	#bit PS0 = OPTION_REG.0
#byte INTCON = 0x0B
	#bit GIE = INTCON.7  //Global Interrupt Enable bit [24Page]
	#bit PEIE = INTCON.6  //Peripheral Interrupt Enable bit
	#bit TMR0IE = INTCON.5

#byte ADCON0 = 0x1F // A/D Control Register 0 (ADCON0)
#byte ADCON1 = 0x9F // A/D Control Register 1 (ADCON1)
	#bit ADON = ADCON0.0 // A/D On bit [127P]
	#bit GODONE = ADCON0.2

	// A/D Conversion Clock Select bits [127P]
	#bit ADCS0 = ADCON0.6 
	#bit ADCS1 = ADCON0.7
	#bit ADCS2 = ADCON1.6

	// A/D Result Format Select bit
	#bit ADFM = ADCON1.7

	// A/D Port Configuration Control bits
	#bit PCFG3 = ADCON1.3
	#bit PCFG2 = ADCON1.2
	#bit PCFG1 = ADCON1.1
	#bit PCFG0 = ADCON1.0

// A/D Result High Register (ADRESH)
// A/D Result Low Register (ADRESL)
#byte ADRESH = 0x1E
#byte ADRESL = 0x9E

//========================================

unsigned long adval = 0;
int cnt = 0;

#int_timer0
void interrupt_1() {

	cnt++;

	if(cnt>=100) {
		TMR0=61;
		clear_interrupt(int_timer0);
		GODONE = 1;
		adval = ADRESH;
		adval = adval << 8;
		adval = adval | ADRESL;
		PORTD = adval >> 2;	
		
		cnt = 0;
	}

}



//========================================

void main (void) {

	TRISD = 0x00;  // Output
	PORTD = 0x00;	// display clear
	ADON = 1;

	// FOSC/
	ADCS2 = 0;
	ADCS1 = 0;
	ADCS0 = 1;

	// Right Justified [132P]
	ADFM = 1;

	// AN0 == A, Vdd-Vss [128P]
	PCFG3 = 0;
	PCFG2 = 0;
	PCFG1 = 0;
	PCFG0 = 0;

	// TMR0 Option [23P]
	T0CS = 0;
	PSA = 0; 
	// TMR0 Rate : 1:256 setup_timer_0(RTCC_INTERNAL | RTCC_DIV_16)
	PS2 = 1;
	PS2 = 1;
	PS0 = 1;

	TMR0 = 61;//set_timer0(x);
	TMR0IE = 1;//enable_interrupts(int_timer0);
	GIE = 1;//enable_interrupts(global);

	while(1) {
		
	}

}