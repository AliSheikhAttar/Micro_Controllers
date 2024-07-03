#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

bool flag = false;
bool password_time = false;
signed char init_password = -1;
int time_second = 0;
int time_minute = 0;
//int time_hour = 0;

char passwords[20][2] = {
	{0x23, 0x32}, {0x11, 0x11}, {0x33, 0x33}, {0x93, 0x69}, {0x99, 0x99},
	{0x77, 0x77}, {0x17, 0x93}, {0x55, 0x25}, {0x41, 0x00}, {0x10, 0x10}
};
int length = 10;

// Function to initialize ports
void init_ports() {
	// Set rows (PC0 - PC3) as output
	DDRC |= 0x0F;  // 0b00001111

	// Set columns (PC4 - PC6) as input with pull-up resistors
	DDRC &= ~0x70;  // 0b10111111
	PORTC |= 0x70;  // 0b01110000

	DDRD = 0xFF;
	DDRB = 0xFF;
	
	// Set PortB0 output
	DDRA = 1;
}

void init_interrupts(){
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << WGM12);
	OCR1A = 15625;
	TIMSK |= (1 << OCIE1A);
	TCCR1B |= (1 << CS11) | (1 << CS10);
	sei();
	PORTB = 0x00; //initial port B
	PORTD = 0x00; //initial port D
}

// Function to scan keypad and return the pressed key
char scan_keypad() {
	// Keypad layout
	const char keys[4][3] = {
		{1, 2, 3},
		{4, 5, 6},
		{7, 8, 9},
		{'*', 0, '#'}
	};

	for (uint8_t row = 0; row < 4; row++) {
		// Drive the current row low and all other rows high
		PORTC = ~(1 << row);

		// Check each column
		for (uint8_t col = 0; col < 3; col++) {
			if (!(PINC & (1 << (col + 4)))) {
				// Wait for key release
				while (!(PINC & (1 << (col + 4))));

				// Return the corresponding key value
				return keys[row][col];
			}
		}
	}
	// No key pressed
	return '\0';
}

int verify_password(char pass[2]){
	for (int i = 0; i<10;i++)
	{
		printf("password[%d] is %s",i, passwords[i]);
		if (pass[0] == passwords[i][0] && pass[1] == passwords[i][1])
		{
			return 1;
		}
	}
	return 0;
}


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
	
	int PORTBL = time_minute % 10;
	int PORTBH = time_minute / 10;
	
	if (!password_time)
	{
		PORTD = PORTDL + 16*PORTDH;
		PORTB = PORTBL + 16*PORTBH;
	}

}



int main(void) {
	// Initialize ports
	init_ports();
	init_interrupts();
	char key;

	while (1) {
		// Scan the keypad
		unsigned char r;
		do
		{
			_delay_ms(20);
			r=PINC&0x0f;
		}while(r==0x0f);
		key = scan_keypad();
		
		if (key != '\0') {
			// Output the key value to Port B (for demonstration purpose)
			if (key == '#') {
				char pass[2];
				pass[0] = PORTB; // Assume PORTA holds the higher byte
				pass[1] = PORTD; // Assume PORTD holds the lower byte
				if (verify_password(pass)){
				PORTA |= 1; // Turn on PORTB pin 0 if password is verified
				_delay_ms(1000);
				PORTA &= 0;
				} 
				else{
					PORTB = 0;
					PORTD = 0;
					_delay_ms(2000);
				}
				//sei();
				password_time = false;
				init_password = -1;
	
			}
			else if(key == '*'){
				strcpy(passwords[length][0], PORTA);
				strcpy(passwords[length][1], PORTD);
				length++;
				PORTD = 0;
				PORTA = 0;
				PORTB = 0;
				//sei();
				password_time = false;
				init_password = -1;
			
			}
			else {
				//cli();
				init_password = init_password == -1 ? 0 : 1;
				if (init_password == 0)
				{
					password_time = true;
					PORTB = 0;
					PORTD = 0;
				}
				PORTB <<= 4;
				PORTB |= (PORTD >> 4);
				PORTD <<= 4;
				PORTD |= (key & 0x0F); // Ensure key is masked correctly
			}

			// Add a small delay to debounce
			_delay_ms(50);
		}
//		else
//			sei();
		
		
	}

	return 0;
}