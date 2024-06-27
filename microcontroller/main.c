/*
 * microcontroller.c
 *
 * Created: 6/5/2024 5:59:05 PM
 * Author : AS
 */ 
//#ifndef F_CPU
//#define F_CPU 8000000UL
//#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

bool flag = false;

ISR(TIMER1_COMPA_vect)
{
	if (flag)
	{
		
		PORTB = 0xAA;
		PORTD = 0xAA;
		flag = false;
	}
	
	else
	{
		PORTB = 0x00;
		PORTD = 0x00;
		flag=true;
	}
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

