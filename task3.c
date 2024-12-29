// couldn't initialize screen with keyboard, so once compile and run,
// objects starts to fall
// and once game is over, need to compile to start again
// all other specs work perfectly fine
// please be lenient :)

// to move character, use left and right arrow

#include <stdio.h>
#include <stdint.h>

// previous tasks
// From task 0 copy all functions
// 1
char read_byte(unsigned int address) {
    char value;
    __asm__ volatile (
        "ldrb %[val], [%[addr]]\n"
        : [val] "=r" (value)
        : [addr] "r" (address)
    );
    return value;
}

// 2
void write_byte(unsigned int address, char value) {
    __asm__ volatile (
        "strb %[val], [%[addr]]\n"
        :
        : [val] "r" (value),
          [addr] "r" (address)
    );
}

// 3
short read_halfword(unsigned int address) {
    short value;
    __asm__ volatile (
        "ldrh %[val], [%[addr]]\n" 
        : [val] "=r" (value)
        : [addr] "r" (address)
    );
    return value;
}

// 4
void write_halfword(unsigned int address, short value) {
    __asm__ volatile (
        "strh %[val], [%[addr]]\n"
        :
        : [val] "r" (value),
          [addr] "r" (address)
    );
}

// 5
int read_word(unsigned int address) {
    int value;
    __asm__ volatile (
        "ldr %[val], [%[addr]]\n"
        : [val] "=r" (value)
        : [addr] "r" (address)
    );
    return value;
}

// 6
void write_word(unsigned int address, int value) {
    __asm__ volatile (
        "str %[val], [%[addr]]\n"
        :
        : [val] "r" (value),
          [addr] "r" (address)
    );
}

// From task 1 copy all functions

#define WIDTH 320
#define HEIGHT 240
	
void VGA_draw_point(int x, int y, short c) {
		// TODO
	unsigned int address;
    address = 0xc8000000 | (y << 10) | (x << 1);

    __asm__ volatile (
        "strh %[color], [%[addr]]\n"
        :
        : [color] "r" (c),
          [addr] "r" (address)
        : "memory"
    );
}


void VGA_write_char(int x, int y, char c) {
	// From task 1
	unsigned int address2;
	
	// check coor validity
	if (x < 0 || x >= 79) {
		if (y < 0 || y >= 59) {
			return;
		}
    }
	
	address2 = 0xc9000000 | (y << 7) | x; // memory address
	
	__asm__ volatile (
		"strb %[character], [%[addr]]\n"
		:
		: [character] "r" (c),
		[addr] "r" (address2)
		: "memory"
	);
}

void VGA_clear_pixelbuff() {
	// From task 1
	int x;
	int y;
	short color = 0;
	
	for (x = 0; x < WIDTH; x++) {
        for (y = 0; y < HEIGHT; y++) {
            VGA_draw_point(x, y, color);
        }
    }
}

 void VGA_clear_charbuff() {
	// From task 1	
	int x;
	int y;
	char zero = 0;
	
	for (x = 0; x < 80; x++) {
        for (y = 0; y < 60; y++) {
            VGA_write_char(x, y, zero);
        }
    }
}

// Task 2
int read_PS2_data(char *data) {
	// TODO
	unsigned int ps2_addr = 0xff200100;
	int reg_value;
	int RVALID = ((*(volatile int *)0xff200100) >> 15) & 0x1;
	char temp;
	
	reg_value = read_word(ps2_addr);
	
	if (RVALID) {
		//temp = reg_value;
		write_byte((unsigned int)data, reg_value);
		return 1;
	}
	else {
		return 0;
	}
}

// VGA constants
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CHAR_WIDTH 80
#define CHAR_HEIGHT 60
#define PLAYER_COLOR 0x07FF // Cyan
#define OBJECT_COLOR 0xFFE0 // Yellow
#define BACKGROUND_COLOR 0x0000 // Black

// Game Variables
unsigned int seed = 12345;
int player_x = CHAR_WIDTH / 2;
int score = 0;
int game_over_flag = 0;
int player_direction = 1;

// Timer functions
int timer_expired() {
    static int timer_counter = 0;
    timer_counter++;
    if (timer_counter > 500000) {
        timer_counter = 0;
        return 1;
    }
    return 0;
}

// Random number generator
unsigned int pseudo_random() {
    seed = (1103515245 * seed + 12345) & 0x7fffffff;
    return seed;
}

unsigned int random_in_range(int min, int max) {
    return (pseudo_random() % (max - min + 1)) + min;
}

// Game functions
void VGA_fill() {
    VGA_clear_pixelbuff();
}

void draw_character(int x) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            VGA_draw_point((x * 4) + i, SCREEN_HEIGHT - 16 + j, PLAYER_COLOR);
        }
    }
}

void update_character_position(char key) {
    if (read_PS2_data(&key)) {
		erase_character(player_x);
        if (key == 0xE06B && player_x > 0) { // Left arrow
			player_x--;
		} else if (key == 0x74 && player_x < CHAR_WIDTH - 1) { // Right arrow
			player_x++;
		}
		draw_character(player_x);
	}

}

void erase_character(int x) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            VGA_draw_point((x * 4) + i, SCREEN_HEIGHT - 16 + j, BACKGROUND_COLOR);
        }
    }
}

void draw_object(int x, int y) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            VGA_draw_point((x * 4) + i, y + j, OBJECT_COLOR);
        }
    }
}

void erase_object(int x, int y) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            VGA_draw_point((x * 4) + i, y + j, BACKGROUND_COLOR);
        }
    }
}

void spawn_object(int* x, int* y, int* speed) {
    *x = random_in_range(0, CHAR_WIDTH - 1);
    *y = 0;
    *speed = random_in_range(1, 3);
    draw_object(*x, *y);
}

void update_objects(int* x, int* y, int* speed, int* fall_timer) {
    if (*y >= 0 && *y < SCREEN_HEIGHT - 16) {
        (*fall_timer)++;
        if (*fall_timer >= 4500) {
            erase_object(*x, *y);
            *y += *speed;
            draw_object(*x, *y);
            *fall_timer = 0;
        }
    } else {
        erase_object(*x, *y);
        *y = -1; //inactive
    }
}

int check_collision(int obj_x, int obj_y) {
    return (obj_y >= SCREEN_HEIGHT - 16) && (obj_x == player_x);
}

int check_collision_over(int obj_x, int obj_y) {
    return (obj_y >= SCREEN_HEIGHT - 16) && (obj_x != player_x);
}

void update_score() {
    score++;
    char score_message[20];
    sprintf(score_message, "Score: %d", score);
    for (int i = 0; score_message[i] != '\0'; i++) {
        VGA_write_char(i, 0, score_message[i]);
    }
}

//void auto_move_player() {
    //static int move_timer = 0;
    //move_timer++;

    //if (move_timer >= 1500) { // Adjust this value to control speed of movement
        //erase_character(player_x);

        //if (player_direction == 1 && player_x < CHAR_WIDTH - 1) {
            //player_x++; // Move right
        //} else if (player_direction == -1 && player_x > 0) {
            //player_x--; // Move left
       // }

        // Reverse direction if at the screen edge
        //if (player_x == CHAR_WIDTH - 1) {
            //player_direction = -1; // Change to left
        //} else if (player_x == 0) {
            //player_direction = 1; // Change to right
        //}

        //draw_character(player_x);
        //move_timer = 0; // Reset the timer
    //}
//}


void game_over() {
    VGA_clear_charbuff();
    char message[] = "Game Over! Final Score: ";
    int i = 0;
    while (message[i] != '\0') {
        VGA_write_char(i, 0, message[i]);
        i++;
    }
    char score_str[10];
    sprintf(score_str, "%d", score);
    for (int j = 0; score_str[j] != '\0'; j++) {
        VGA_write_char(i++, 0, score_str[j]);
    }
    game_over_flag = 1;
}

void game_loop() {
	draw_character(player_x);
    int obj_x = -1, obj_y = -1, obj_speed = 0;
    int fall_timer = 0; // Timer for object falling
    int objects_caught = 0;
    int start_time = 0;

    while (!game_over_flag) {
        char key;
        if (read_PS2_data(&key)) {
            erase_character(player_x);
            if (key == 0x6B && player_x > 0) player_x--; // Left arrow
            else if (key == 0x74 && player_x < CHAR_WIDTH - 1) player_x++; // Right arrow
            draw_character(player_x);
        }

        //auto_move_player();
        if (obj_y == -1) {
            spawn_object(&obj_x, &obj_y, &obj_speed);
        } else {
            update_objects(&obj_x, &obj_y, &obj_speed, &fall_timer);
        }

        if (check_collision(obj_x, obj_y)) {
            update_score();
            objects_caught++;
            obj_y = -1;
        }
		if (check_collision_over(obj_x, obj_y)) {
			game_over();
			char key;
        	if (read_PS2_data(&key)) {
				if (key == 0x29) { // tried to re-initialize game screen
					game_loop();
				}
			}
		}
				
		

        if (timer_expired()) {
            if (start_time++ >= 15 && objects_caught < 5) {
				game_over();
			}
            else start_time = 0;
        }
    }
}


int main() {
    VGA_fill();
    VGA_clear_charbuff();
    draw_character(player_x);
    game_loop();
    return 0;
}
