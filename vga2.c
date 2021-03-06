// EEC181A, Lab 3, Assignment 210
// Eduard Alfonso
// Jack Chen
// Jimmy Yu

#include <stdlib.h> // For random()

#define SPEED	2

// Function to send a string of text to the VGA monitor
void VGA_text(int x, int y, char * text_ptr) {
	int offset;
	
	// VGA char_buffer_slave location address - Base address is 0x04004000 in Qsys
	volatile char * character_buffer = (char *) 0xC4004000;
	
	// Note: Assumes all characters will fit on a single line
	offset = (y << 7) + x;
	while ( *(text_ptr) )
	{
		*(character_buffer + offset) = *(text_ptr);		// write to the character buffer
		++text_ptr;
		++offset;
	}
}

// Function to Draw a filled rectangle on the VGA monitor
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color) {
	int offset, row, col;
	
	// VGA pixel buffer location address in On-Chip Memory Address in Qsys//
	// Remember Pixel DMA does not have embedded memory
	// VGA SDRAM location address - Base Address is 0x04000000 in Qsys
	volatile short * pixel_buffer = (short *) 0xC0000000;
	
	// Note: Assume that the box coordinates are valid on the screen
	// If coordinates are off the screen it is helpful to set the coordinates so that
	// the box shows on the screen again. You may need to choose coordinates within the
	// screen resolution
	// Each pixel value is represented as 16-bit halfword, with five bits for the blue,
	// five bits for the red, and six bits for green
	
	for (row = y1; row <= y2; row++)
		for (col = x1; col <= x2; ++col)
		{
			offset = (row << 9) + col;
			*(pixel_buffer + offset) = pixel_color; // compute halfword address, set pixel
		}
}

// Call this to sleep for sec seconds
void sleep(int sec) {
	int i = 0;
	for (i = 0; i <= sec * 2500000/SPEED; i++) {}
}

void draw_pause(void) {
	// Text variables
	char title_lab[30] = "EEC 181A, Lab 3 Assignment 2\0";
	char title_team[12] = "Team Yay!\0";
	
	// Draw a pause button
	sleep(1);
	VGA_box (0, 00, 320, 240, 0x1878);
	VGA_box(95, 60, 225, 180, 0x0000);
	VGA_box(130, 80, 150, 160, 0xFFFF);
	VGA_box(170, 80, 190, 160, 0xFFFF);
	VGA_text (1, 1, title_lab);
	VGA_text (1, 2, title_team);
}

int main(void) {
	// Board Mapping
	volatile int * sw		= (int * ) 0xFF200070; // SW
	volatile int * hexp1	= (int * ) 0xFF200060; // HEX2_HEX0
	volatile int * hexp2	= (int * ) 0xFF200050; // HEX5_HEX4_HEX3
	volatile int * leds		= (int * ) 0xFF200040; // LEDR
	volatile int * right	= (int * ) 0xFF200030; // KEY0
	volatile int * down		= (int * ) 0xFF200000; // KEY1
	volatile int * up		= (int * ) 0xFF200010; // KEY2
	volatile int * left		= (int * ) 0xFF200020; // KEY3	
		
	// Zero buffers
	*((short *) 0xC0000000) = 0;
	*((char *) 0xC4004000) = 0;
	// Zero PIO
	*(leds) = 0x0;
	
	// Text variables
	char title_lab[30] 	= "EEC 181A, Lab 3 Assignment 2\0";
	char title_team[12] = "Team Yay!\0";

	char p1_begin[30] 	= "Player 1's turn!\0";
	char p2_begin[30] 	= "Player 2's turn!\0";
	
	char p1_win[17] 	= "Player 1 wins!\0";
	char p2_win[17]		= "Player 2 wins!\0";
	
	char title_tie[13] 	= "Tie game!\0";
	char reset[10]		= "Resetting...               \0";
	
	char blank[30] 		= "                           \0";
	
	// Box variables/configuration
	int box_shift = 10;
	int box_size = 30;
	
	// Box positions
		int box1pos_x; 
		int box1pos_y; 
		int box2pos_x; 
		int box2pos_y; 
	
	int pRight = *(right);
	int pDown = *(down);
	int pUp = *(up);
	int pLeft = *(left);
	
	// p1/p2 variables
	int p1_time;
	int p2_time;

	VGA_text (1, 3, blank);
	draw_pause();
	
	int game = 0;
	
	while(1) {
		if ((*(sw) == 4) || (*(sw) == 5) ||
			(*(sw) == 6) ||  (*(sw) == 7) ) {
			// Reset stuff
			p1_time = 0;
			p2_time = 0;
			game = 0;
			*(leds) = 0;
			
			// Print resetting
			
			VGA_text (1, 3, reset);
			
			// Randomize box positions
			box1pos_x = (rand() % 32)*10; 	// X -> 0 to 320
			box1pos_y = (rand() % 24)*10; 	// Y -> 0 to 240
			do { // Regenerate if box2x == box1x
				box2pos_x = (rand() % 32)*10;
			} while (box2pos_x == box1pos_x);
			do { // Regenerate if box1x == box1y
				box2pos_y = (rand() % 24)*10;;
			} while (box2pos_y == box1pos_y);
		
			// Re-draw background
			VGA_box (0, 00, 320, 240, 0x1878);
			// Draw green box
			VGA_box(box1pos_x, box1pos_y, box1pos_x+box_size, box1pos_y+box_size, 0xF800);
			// Draw red box
			VGA_box(box2pos_x, box2pos_y, box2pos_x+box_size, box2pos_y+box_size, 0x0FB0);
		} else { 
			// Play game
			switch(game) {
			case(0): // Initial pause
				VGA_text (1, 3, p1_begin);
				draw_pause();
				
				// Wait until P1 presses a SW
				if (((*(right) != pRight) || (*(down) != pDown) ||
					(*(up) != pUp) || (*(left) != pLeft))) {
						game = 1;
						// Randomize box positions
						box1pos_x = (rand() % 32)*10; 	// X -> 0 to 320
						box1pos_y = (rand() % 24)*10; 	// Y -> 0 to 240
						do { // Regenerate if box2x == box1x
							box2pos_x = (rand() % 32)*10;
						} while (box2pos_x == box1pos_x);
						do { // Regenerate if box1x == box1y
							box2pos_y = (rand() % 24)*10;;
						} while (box2pos_y == box1pos_y);
					
						// Reset P1 timer
						p1_time = 0;
					}
			break;
			case(1): // P1 turn
					if((box1pos_x == box2pos_x) && (box1pos_y == box2pos_y)) {
						// P1 finished
						game = 2;
						VGA_text (1, 3, blank);
					}
			
					VGA_text (1, 3, p1_begin);
			
					// Movement
					switch( *(sw) ) {
					case(1): // Move box 1
						if ( (*(right) != pRight) && (box1pos_x < 290) ) {	// Move right if not at edge of screen
							box1pos_x += box_shift;
						} if ( (*(down) != pDown) && (box1pos_y < 210) ) {	// Move down if not at edge
							box1pos_y += box_shift;
						} if ( (*(up) != pUp) && (box1pos_y > 0) ) {		// Move up if not at edge
							box1pos_y -= box_shift;
						} if ( (*(left) != pLeft) && (box1pos_x > 0) ) {	// Move left if not at edge
							box1pos_x -= box_shift;
						} break;
					case(2): // Move box 2
						if ( (*(right) != pRight) && (box2pos_x < 290) ) {	// Move right if not at edge of screen
							box2pos_x += box_shift;
						} if ( (*(down) != pDown) && (box2pos_y < 210) ) {	// Move down if not at edge
							box2pos_y += box_shift;
						} if ( (*(up) != pUp) && (box2pos_y > 0) ) {		// Move up if not at edge
							box2pos_y -= box_shift;
						} if ( (*(left) != pLeft) && (box2pos_x > 0) ) {	// Move left if not at edge
							box2pos_x -= box_shift;
						} break;
					} // switch(sw)
					
					// Re-draw background
					VGA_box (0, 00, 320, 240, 0x1878);
					// Draw green box
					VGA_box(box1pos_x, box1pos_y, box1pos_x+box_size, box1pos_y+box_size, 0xF800);
					// Draw red box
					VGA_box(box2pos_x, box2pos_y, box2pos_x+box_size, box2pos_y+box_size, 0x0FB0);
					
					pRight = *(right);
					pDown = *(down);
					pUp = *(up);
					pLeft = *(left);
					*(hexp1) = p1_time++/SPEED;
			break;
			case(2): // Pause
					VGA_text (1, 3, p2_begin);
					draw_pause();
				
					// Wait until P1 presses a SW
					if (!((*(right) == pRight) && (*(down) == pDown) &&
						(*(up) == pUp) && (*(left) == pLeft))) {
							game = 3;
							// Randomize box positions
							box1pos_x = (rand() % 32)*10; 	// X -> 0 to 320
							box1pos_y = (rand() % 24)*10; 	// Y -> 0 to 240
							do { // Regenerate if box2x == box1x
								box2pos_x = (rand() % 32)*10;
							} while (box2pos_x == box1pos_x);
							do { // Regenerate if box1x == box1y
								box2pos_y = (rand() % 24)*10;;
							} while (box2pos_y == box1pos_y);
						
							// Reset P2 timer
							p2_time = 0;
					}
			
			break;
			case(3): // P2
					if((box1pos_x == box2pos_x) && (box1pos_y == box2pos_y)) {
						// P2 finished
						game = 4;
						VGA_text (1, 3, blank);
					}
			
					VGA_text (1, 3, p2_begin);
			
					// Movement
					switch( *(sw) ) {
					case(1): // Move box 1
						if ( (*(right) != pRight) && (box1pos_x < 290) ) {	// Move right if not at edge of screen
							box1pos_x += box_shift;
						} if ( (*(down) != pDown) && (box1pos_y < 210) ) {	// Move down if not at edge
							box1pos_y += box_shift;
						} if ( (*(up) != pUp) && (box1pos_y > 0) ) {		// Move up if not at edge
							box1pos_y -= box_shift;
						} if ( (*(left) != pLeft) && (box1pos_x > 0) ) {	// Move left if not at edge
							box1pos_x -= box_shift;
						} break;
					case(2): // Move box 2
						if ( (*(right) != pRight) && (box2pos_x < 290) ) {	// Move right if not at edge of screen
							box2pos_x += box_shift;
						} if ( (*(down) != pDown) && (box2pos_y < 210) ) {	// Move down if not at edge
							box2pos_y += box_shift;
						} if ( (*(up) != pUp) && (box2pos_y > 0) ) {		// Move up if not at edge
							box2pos_y -= box_shift;
						} if ( (*(left) != pLeft) && (box2pos_x > 0) ) {	// Move left if not at edge
							box2pos_x -= box_shift;
						} break;
					} // switch(sw)
					
					// Re-draw background
					VGA_box (0, 00, 320, 240, 0x1878);
					// Draw green box
					VGA_box(box1pos_x, box1pos_y, box1pos_x+box_size, box1pos_y+box_size, 0xF800);
					// Draw red box
					VGA_box(box2pos_x, box2pos_y, box2pos_x+box_size, box2pos_y+box_size, 0x0FB0);
					
					pRight = *(right);
					pDown = *(down);
					pUp = *(up);
					pLeft = *(left);
					*(hexp2) = p2_time++/SPEED;
			
					
			
			break;
			case(4): // Decide winner
					VGA_text (1, 3, blank);
					draw_pause();
					
					// Display winner
					if (p2_time > p1_time) {
						*(leds) = 60;
						VGA_text(1, 3, p1_win);
					} else if (p1_time > p2_time) {
						*(leds) = 960;
						VGA_text(1, 3, p2_win);
					} else {
						*(leds) = 1023;
						VGA_text(1, 3, title_tie);
					}
					
			break;
			default: 
			game = 0;
			
				
			} // Switch
		}
		
		sleep(1);
	}
}
