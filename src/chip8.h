#ifndef CHIP8_H
#define CHIP8_H

#include "util.h"

#define ERR_OK 0
#define ERR_INVALID_PROGRAM_SIZE 1
#define ERR_INVALID_OPCODE 2

class Chip8 {
protected:
	u08 	memory[4096];
	u16 	opcode;
	u08		V[16];
	u16		I;
	u16		pc;
	u08		display[64 * 32];
	u08		delay_timer;
	u08		sound_timer;
	u16		stack[16];
	u16		stack_pointer;
	u08		key[16];
	u08		draw_flag;

public:
	int load_program(u08 *buffer, u16 size);
	void init();
	void cycle();
	void update_timers();
	void set_key(int idx, bool enabled);
	u08 *get_display() { return &display[0]; }
	bool should_draw() const { return draw_flag > 0; }

	Chip8();
	~Chip8();
};

#endif // CHIP8_H