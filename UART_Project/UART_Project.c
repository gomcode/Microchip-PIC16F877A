#include <16F877A.h>
#fuses HS,NOPROTECT,NOBROWNOUT,NOWDT,NOPUT,NOCPD,NOLVP,NOWRT
#use delay(clock=20000000)

#byte PORTD = 0x08
#bit TEST_PIN = PORTD.0
#byte TRISD = 0x88	// Input or Output

#byte INTCON = 0x0B
#bit GIE = INTCON.7  //Global Interrupt Enable bit [24Page]
#bit PEIE = INTCON.6  //Peripheral Interrupt Enable bit
#bit TMR0IE = INTCON.5

/*

void main(void){
	TRISD = 0x00;  // Output
	PORTD = 0x00;	// clear

	while(1){

		// LED Blink - 1Hz
		PORTD = 0xff;
		delay_ms(500);
		PORTD = 0x00;
		delay_ms(500);

		// LED Blink - 2Hz
		PORTD = 0xff;
		delay_ms(0);
		PORTD = 0x00;
		delay_ms(
0);

		
	}	
}

*/
#if 1
int cnt =0;
#int_timer0
void interrupt() {

		clear_interrupt(int_timer0);
//		set_timer0(255);

//		PORTD = 0xff;
//		delay_ms(100);
//		PORTD = 0x00;
//		delay_ms(100);


	if(cnt++ == 100){
	//	output_toggle(PIN_D0);
		PORTD = ~PORTD; //된다!
		cnt = 0;
	}
//	cnt++;
//	if(cnt == 1) output_high(PIN_D0);
//	else if(cnt == 50)output_low(PIN_D0);
//	else if(cnt >= 100) cnt = 0;
}


void main(void) {

	TRISD = 0x00;  // Output
	PORTD = 0x00;	// clear

//	GIE = 1;
//	PEIE = 1;


	setup_timer_0(RTCC_INTERNAL | RTCC_DIV_256);
	set_timer0(100);
	TMR0IE = 1;//enable_interrupts(int_timer0);
	GIE = 1;//enable_interrupts(global);
	
//	clear_interrupt(int_timer0);

	while(1);
}
#endif

#if 0
int count = 0;
# INT_RTCC
RTCC_ISR(){
	count++;
}

void main(void){
	
	TRISD = 0x00;
	PORTD = 0x00;

	setup_counters(rtcc_internal, rtcc_div_256);
	enable_interrupts(int_rtcc);
	enable_interrupts(global);

	while(1){

		if(count == 76){
			count = 0;

			PORTD = 0xff;
			delay_ms(200);
			PORTD = 0x00;
		}
	}	
}
#endif