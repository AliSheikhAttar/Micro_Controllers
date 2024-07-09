#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

bool flag = false;
bool password_time = false;
signed char init_password = -1;

int time_second = 0;
int time_minute = 0;
int time_hour = 0;

char passwords[20][2] = {
	{0x23, 0x32}, {0x11, 0x11}, {0x33, 0x33}, {0x93, 0x69}, {0x63, 0x63},
	{0x77, 0x77}, {0x17, 0x93}, {0x55, 0x25}, {0x41, 0x00}, {0x10, 0x10}
};
int length = 10;

typedef struct {
	char pass[3];  // Array to hold 2 characters + null terminator
	char timestamp[4];  // Array to hold the time string (HHMMSS + null terminator)
} Entry;

Entry logs[100];
int logs_ctr = 0;

// Function to initialize ports
void init_ports() {
	// Set rows (PC0 - PC3) as output
	DDRC |= 0x0F;  // 0b00001111

	// Set columns (PC4 - PC6) as input with pull-up resistors
	DDRC &= ~0x70;  // 0b00011111
	PORTC |= 0x70;  // 0b01110000

	DDRD = 0xFF;
	DDRB = 0xFF;
	DDRA = 0xFF;
	DDRC |= 0x80;
}

void init_interrupts(){
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << WGM12);
	OCR1A = 12625;
	//12500
	//OCR1A = 125;
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
		{'*', 0xA, '#'}
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
	for (int i = 0; i<length;i++)
	{
		if (pass[0] == passwords[i][0] && pass[1] == passwords[i][1])
		{
			return 1;
		}
	}
	return 0;
}

void flaging(char hex, bool all){
	if (all)
	{
		PORTA = hex; PORTB = hex; PORTD = hex;
		_delay_ms(900);
		PORTA = 0; PORTB = 0; PORTD = 0;
	}
	else{
		PORTA = hex;
		_delay_ms(900);
		PORTA = 0;
	}
}

void log_record(int hour_t, int minute_t, int second_t) {
	// password
	logs[logs_ctr].pass[0] = PORTB; logs[logs_ctr].pass[1] = PORTD; logs[logs_ctr].pass[2] = '\0';
	/*
    time_t raw_time;
    struct tm *time_info;
	// Get the current time
	time(&raw_time);

	// Convert the time to local time
	time_info = localtime(&raw_time);
	//strftime(logs[logs_ctr].timestamp, sizeof(logs[logs_ctr].timestamp), "%H%M%S", t);
	*/
	logs[logs_ctr].timestamp[0] = hour_t %256;
	logs[logs_ctr].timestamp[1] = minute_t %256;
	logs[logs_ctr].timestamp[2] = second_t %256;
	logs[logs_ctr].timestamp[3] = '\0';
	logs_ctr++;
}


bool log_command(char pass[2]){
	if(pass[0] == 0x99 && pass[1] == 0x99) //equal 9999
		return true;
	return false;
}

bool verify_log(){
	if ( logs_ctr> 1 && logs[logs_ctr-1].pass[0] == 0x11 && logs[logs_ctr-1].pass[1] == 0x11){
		return true;
	}
	return false;
}

void display_log(){
	for(int i = 0;i<logs_ctr;i++){
		int sec_t = logs[i].timestamp[2] ;
		int min_t = logs[i].timestamp[1] ;
		int hour_t = logs[i].timestamp[0] ;
		PORTA = ((hour_t /10)*16 + hour_t % 10);
		PORTB = ((min_t /10)*16 + min_t % 10);
		PORTD = ((sec_t /10)*16 + sec_t % 10);
		_delay_ms(1000);
		flaging(0xAA, true);
		PORTA = i + 1;
		PORTB = logs[i].pass[0];
		PORTD = logs[i].pass[1];
		_delay_ms(2000);
		
if(i < (logs_ctr-1)){
PORTA = 0x88;
PORTB = 0x88;
PORTD = 0x88;
}
else{
	PORTA = 0xFF;
	PORTB = 0x11;
	PORTD = 0x55;
}
		_delay_ms(1000);
		
		
	}
}


bool set_password(){
	char new_pass[2];
	new_pass[0] = PORTB;
	new_pass[1] = PORTD;
	//passwords[length][1] = PORTD;
	for (int i =0;i<length;i++)
	{
		if ((passwords[i][0] == new_pass[0]) && (passwords[i][1] == new_pass[1]))
		{
			return false;
		}
	}
	passwords[length][0] = PORTB;
	passwords[length][1] = PORTD;
	length++;
	return true;
					
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
bool minute_check(){
	if (time_minute < 59){
		time_minute += 1;
		return false;
	}
	else{ time_minute = 0; return true; }
}

void hour_check(){
	if (time_hour < 23){
		time_hour += 1; 
	}
	else{ time_hour = 0; }
}

ISR(TIMER1_COMPA_vect)
{
	
	if (second_check())
		if(minute_check())
			hour_check();

	
	int PORTDL = time_second % 10;
	int PORTDH = time_second / 10;
	
	int PORTBL = time_minute % 10;
	int PORTBH = time_minute / 10;
	
	int PORTAL = time_hour % 10;
	int PORTAH = time_hour / 10;
	
	if (!password_time)
	{
		PORTD = PORTDL + 16*PORTDH;
		PORTB = PORTBL + 16*PORTBH;
		PORTA = PORTAL + 16*PORTAH;
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
			if (key == '#' && password_time) {
				char pass[2];
				pass[0] = PORTB; // Assume PORTA holds the higher byte
				pass[1] = PORTD; // Assume PORTD holds the lower byte
				if (log_command(pass))
				{
					if(verify_log()){
						display_log();
					}
					else{
						flaging(0xCC,true);
					}
					password_time = false;
					init_password = -1;
					
				}
				else{
					if (verify_password(pass)){
						PORTA |= 1; // Turn on PORTB pin 0 if password is verified
						log_record(time_hour, time_minute, time_second);
						_delay_ms(500);
					}
					else{
						PORTB = 0;
						PORTD = 0;
						PORTA = 0xCC;
						_delay_ms(1000);
					}
					//sei();
					password_time = false;
					init_password = -1;
				}
	
			}
			else if(key == '*'){
				if(set_password())
					flaging(0xAA,true);
				else(flaging(0xCC,true));
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
				if (key == 0xA)
				{
					key = 0;
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