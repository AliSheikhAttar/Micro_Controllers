/*
  microcontroller.c
 
  Created: 6/5/2024 5:59:05 PM
  Author : AS
 */ 
//#ifndef F_CPU
//#define F_CPU 8000000UL
//#endif
/*
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

bool flag = false;

int time_second = 0;
int time_minute = 0;
//int time_hour = 0;

bool second_check(){
	if ( time_second < 59 ){
		time_second += 1;
		return false;
	}
	else{
		time_second = 0;
		return true;
	}

}
void minute_check(){
	if (time_minute < 59){
		time_minute += 1;
	}
	else{ time_minute = 0; }
}

ISR(TIMER1_COMPA_vect)
{
	
	if (second_check()){
		minute_check();
	}
	
	int PORTDL = time_second % 10;
	int PORTDH = time_second / 10;
	PORTD = PORTDL + 16*PORTDH;
	
	int PORTBL = time_minute % 10;
	int PORTBH = time_minute / 10;
	PORTB = PORTBL + 16*PORTBH;

}


int main(void)
{
	
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << WGM12);
	OCR1A = 15625; 
	TIMSK |= (1 << OCIE1A);
	TCCR1B |= (1 << CS11) | (1 << CS10);
	 
	DDRB = 0xFF;  //make port B output
	DDRD = 0xFF;  // make port D output

	sei();

	PORTB = 0x00; //initial port B
	PORTD = 0x00; //initial port D
	while (1) // main program
	{
	}
	return 0;
}


*/