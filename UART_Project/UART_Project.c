#include <16F877A.h>
#include <stdio.h>
#include <stdlib.h>

#fuses HS,NOPROTECT,NOBROWNOUT,NOWDT,NOPUT,NOCPD,NOLVP,NOWRT
#use delay(clock=20000000)
#use rs232(UART1,baud=115200,parity=N,bits=8)
#byte TRISA = 0x85
	#bit RA1_Data = TRISA.1
#byte TRISD = 0x88 // IO Mode
#byte PORTD = 0x08 // IO Port

#byte TMR0 = 0x01
#byte OPTION_REG = 0x81
	#bit T0CS = OPTION_REG.5
	#bit PSA = OPTION_REG.3
	#bit PS2 = OPTION_REG.2
	#bit PS1 = OPTION_REG.1
	#bit PS0 = OPTION_REG.0

#byte TRISE = 0x89
	#bit TRISEbit1 = TRISE.1 
	#bit TRISEbit0 = TRISE.0

#byte TMR1L = 0x0E
#byte TMR1H = 0x0F
#byte T1CON = 0x10 //TIMER1 CONTROL REGISTER
	#bit T1CKPS1 = T1CON.5
	#bit T1CKPS0 = T1CON.4
	#bit TMR1CS = T1CON.1
	#bit TMR1ON = T1CON.0
#byte PIE1 = 0x8C
	#bit TMR1IE = PIE1.0

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



/*	========================================
	=              Parameters              =
	========================================   */
	unsigned long thres = 0xFF;
	unsigned long tmr1P; // tmr1 period

unsigned long int_cnt1 = 0;
unsigned long int_cnt2 = 0;
unsigned long adval = 0;
unsigned int bpm = 0;

unsigned int val1 = 0;
unsigned int val2 = 0;
unsigned int val3 = 0;
unsigned int isPeakPoint = 0;

unsigned int isSeg7Left = 1;
unsigned int seg7L = 0x0;
unsigned int seg7R = 0x0;



// 480Hz Timer //
void T480() {
	TMR1H = 0b11010111; // Timer = 55119 Initialization
	TMR1L = 0b01001111;	
}



// 360Hz Timer //
void T360() {
	TMR1H = 0b11001001; // Timer = 51647 Initialization
	TMR1L = 0b10111111;	
}



// UART Terminal Printer //
void UART (int prtin) {
	printf("BPM = %u" ,prtin);
	printf("\n");
}



// Num.Value to 7-SegmentValue Converter //
unsigned int seg7_conv(unsigned int input) {

	switch (input) {
	case 0:
		return 0x3F;
		break;
	case 1:
		return 0x06;
		break;
	case 2:
		return 0x5B;
		break;
	case 3:
		return 0x4F;
		break;
	case 4:
		return 0x66;
		break;
	case 5:
		return 0x6D;
		break;
	case 6:
		return 0x7D;
		break;
	case 7:
		return 0x27;
		break;
	case 8:
		return 0x7F;
		break;
	case 9:
		return 0x6F;
		break;
	default :
		return 0x00;
	} 
	
}



// for 7-Segment LED Display Management //
void seg7_dsp() {

	if(isSeg7Left==1) {
		PORTD = seg7L; // to Send converted seg7L value to PORTD before turing on
		TRISEbit1 = 0; // for turing on 7-Segment LEFT digit
		TRISEbit0 = 1; // "
		isSeg7Left=0; // EXIT 'if(isSeg7Left==1)'
	} else {
		PORTD = seg7R; // to Send converted seg7R value to PORTD before turing on
		TRISEbit1 = 1; // for turing on 7-Segment RIGHT digit
		TRISEbit0 = 0; // "
		isSeg7Left = 1; // ENTER 'if(isSeg7Left==1)' 
	}

	if(adval>thres) {
		PORTD |= 0b10000000; // to Turn ON 7-Segment dot(.) @Peakpoint
	} else PORTD &= 0b01111111; // to Turn OFF 7-Segment dot(.) !@Peakpoint

}



// for Sending Data to 7-Segment System //
void seg7(unsigned int input) {

	if(input>99) seg7L=seg7R=0b01000000;
	else {
		seg7L = seg7_conv((input/10));
		seg7R = seg7_conv(input%10);
	}

}



// to Filter 8-point(60Hz) Period Power Noise //
unsigned long arr_adval[8] = {0};
unsigned long sum = 0;
int i = 0;
void filter_8p(unsigned long input) {

	for(i=0; i<7; i++) {
		arr_adval[i] = arr_adval[i+1];
		sum += arr_adval[i];
	}

	i = 0; // Reinitialize i
	arr_adval[7] = input;
	sum += arr_adval[7];
	sum /= 8; // Calculate Average
	adval = sum;
	sum = 0;

}



void bpm_conv(unsigned int input) {

	int_cnt1++; // interrupt xHz일 때 1초에 x씩 증가
	
	if ( thres<adval ) {
		val1 = val2;
		val2 = val3;
		val3 = input;
		if ( val2>=val1 ) {
			if ( val3<val2 ) {
				if ( isPeakPoint==0 ) {
					bpm =(60*tmr1P/int_cnt1);
	/*				if (int_cnt1%8<6) {
						sum += (60*tmr1P/int_cnt1);
					} else {
						bpm = sum/8;
					}    */
					seg7(bpm);
					int_cnt1=0;
					isPeakPoint=1;
				}
			}	
		}
	} else {
		isPeakPoint=0;
	}
	
}



// to Control proper heartbeat-Threshold Adaptively  //
unsigned long avr_adval[8] = {0};
unsigned long advalMax = 0x00;
unsigned long advalMin = 0xFF;
void thresAdaptiveControl(unsigned long adval) {
	int_cnt2++;
	if(int_cnt2<tmr1P) { // Determine Minimum BPM ==
		if(advalMax < adval) {
			advalMax = adval;
		}
		if(advalMin > adval) {
			advalMin = adval;	
		}
	} else {
		if(advalMax>advalMin) thres = ( (advalMax+advalMin)*8/14 + 40 );
		advalMax = 0x0000;
		advalMin = 0xFF;
		int_cnt2 = 0;
	}
}



#int_timer1
void adval_conv() {

/*	========================================
	=            TimerSettings             =
	========================================   */

	T360();

GODONE = 1; // for making ADC to run continuously
//ADRESH = ---- --xx, ADRESL = xxxx xxxx -> send upper 8bit to adval
adval = ADRESH;
adval = adval << 8;
adval = adval | ADRESL;	

	filter_8p(adval);
	thresAdaptiveControl(adval);
	bpm_conv(adval);

	putc(0xFF);
	putc(0xFF);
	putc((unsigned char)(adval>>8));
	putc((unsigned char)adval);
	putc((unsigned char)(thres>>8));
	putc((unsigned char)thres);
	putc(0x00);
	putc(bpm);
	putc(bpm);

	seg7_dsp(); //7-Segment LED Management
	clear_interrupt(int_timer1); //Exit Interrupt

}



void main (void) {

	TRISD = 0x00;  // Output
	PORTD = 0x00;	// display clear
	ADON = 1; // A/D ON bit

	// A/D Conversion Clock Select bits [127P]
	ADCS2 = 0;
	ADCS1 = 0;
	ADCS0 = 1;

	// ADC Result Right Justified [132P]
	ADFM = 1;

	// A/D Port Configuartion Control bits : AN0 == A, Vdd-Vss [128P]
	PCFG3 = 0;
	PCFG2 = 0;
	PCFG1 = 0;
	PCFG0 = 0;

	// TMR1 Interrupt Option [23P]
	TMR1ON = 1; 
	TMR1CS = 0;
	TMR1IE = 1; // enable_interrupts(int_timer1);
	GIE = 1; //	enable_interrupts(global); 
	PEIE = 1;

	// TMR1 Rate : 1:1 
	T1CKPS1 = 0;
	T1CKPS0 = 0;

	tmr1P = 360; // tmr1 360Hz period; 
	T360();
	while(1) { 	}

}