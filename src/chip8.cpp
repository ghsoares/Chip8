#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

u08 chip8_fontset[80] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

int Chip8::load_program(u08 *buffer, u16 size) {
	if (size > 0x0E00) {
		return ERR_INVALID_PROGRAM_SIZE;
	}
	init();
	for (u16 i = 512; i < 0x0F00; i++) {
		if (i - 512 < size) {
			memory[i] = buffer[i - 512];
		} else {
			memory[i] = 0;
		}
	}
	return ERR_OK;
}

void Chip8::init() {
	for (u16 i = 0; i < 4096; i++)
		memory[i] = 0;

	for (u16 i = 0; i < 80; i++)
		memory[i] = chip8_fontset[i];

	for (u16 i = 0; i < 16; i++) {
		V[i] = 0;
		stack[i] = 0;
		key[i] = 0;
	}

	for (u16 i = 0; i < 64 * 32; i++)
		display[i] = 0;

	opcode = 0;
	I = 0;
	pc = 0x200;
	delay_timer = 0;
	sound_timer = 0;
	stack_pointer = 0;
	draw_flag = 0;

	srand(time(nullptr));
}

void Chip8::cycle() {
	draw_flag = 0;

	// Fetch
	opcode = (memory[pc] << 8) | (memory[pc + 1] << 0);

	u08 x = (opcode >> 8) & 0xF;
	u08 y = (opcode >> 4) & 0xF;
	u08 n = opcode & 0x000F;
	u08 nn = opcode & 0x00FF;
	u16 nnn = opcode & 0x0FFF;

	// Decode
	switch (opcode & 0xF000) {
		case 0x0000: {
			switch (opcode & 0x00FF) {
				case 0x00E0: {
					// Execute
					for (u16 i = 0; i < 64 * 32; i++) {
						display[i] = 0; 
					}
					draw_flag = 1;
					pc += 2;
				} break;

				case 0x00EE: {
					stack_pointer--;
					pc = stack[stack_pointer];
					pc += 2;
				} break;

				default:
					std::cerr << "Invalid opcode: " << std::hex << opcode << " at: " << pc << std::endl;
			}
		} break;

		case 0x1000: {
			// Execute
			pc = nnn;
		} break;

		case 0x2000: {
			// Execute
			stack[stack_pointer] = pc;
			stack_pointer++;
			pc = nnn;
		} break;

		case 0x3000: {
			// Execute
			if (V[x] == nn)
				pc += 2;
			pc += 2;
		} break;

		case 0x4000: {
			// Execute
			if (V[x] != nn)
				pc += 2;
			pc += 2;
		} break;

		case 0x5000: {
			// Execute
			if (V[x] == V[y])
				pc += 2;
			pc += 2;
		} break;

		case 0x6000: {
			// Execute
			V[x] = nn;
			pc += 2;
		} break;

		case 0x7000: {
			// Execute
			V[x] += nn;
			pc += 2;
		} break;

		case 0x8000: {
			switch (opcode & 0x000F) {
				case 0x0000: {
					// Execute
					V[x] = V[y];
					pc += 2;
				} break;

				case 0x0001: {
					// Execute
					V[x] |= V[y];
					pc += 2;
				} break;

				case 0x0002: {
					// Execute
					V[x] &= V[y];
					pc += 2;
				} break;

				case 0x0003: {
					// Execute
					V[x] ^= V[y];
					pc += 2;
				} break;

				case 0x0004: {
					// Execute
					V[0xF] = V[x] > 0xFF - V[y] ? 1 : 0;
					V[x] += V[y];
					pc += 2;
				} break;

				case 0x0005: {
					// Execute
					V[0xF] = V[x] <= V[y] ? 0 : 1;
					V[x] = V[x] - V[y];
					pc += 2;
				} break;

				case 0x0006: {
					// Execute
					V[0xF] = V[x] & 0x1;
					V[x] = V[x] >> 1;
					pc += 2;
				} break;

				case 0x0007: {
					// Execute
					V[0xF] = V[y] <= V[x] ? 0 : 1;
					V[x] = V[y] - V[x];
					pc += 2;
				} break;

				case 0x000E: {
					// Execute
					V[0xF] = (V[x] >> 7) & 0x1;
					V[x] = V[x] << 1;
					pc += 2;
				} break;

				default:
					std::cerr << "Invalid opcode: " << opcode << std::endl;
			}
		} break;

		case 0x9000: {
			// Execute
			if (V[x] != V[y])
				pc += 2;
			pc += 2;
		} break;

		case 0xA000: {
			// Execute
			I = nnn;
			pc += 2;
		} break;

		case 0xB000: {
			// Execute
			pc = V[0x0] + nnn;
		} break;

		case 0xC000: {
			// Execute
			V[x] = (rand() % 0xFF) & nn;
			pc += 2;
		} break;

		case 0xD000: {
			// Execute
			u08 px = V[x];
			u08 py = V[y];

			V[0xF] = 0;
			for (u08 j = 0; j < n; j++) {
				u08 spr = memory[I + j];
				for (u08 i = 0; i < 8; i++) {
					u16 disIdx = (py + j) * 64 + (px + i);
					u08 sprPix = (spr >> (7 - i)) & 0x1;
					if (sprPix > 0) {
						u08 disPix = display[disIdx];
						u08 newPix = disPix ^ sprPix;
						V[0xF] |= newPix == 0 ? 1 : 0;
						display[disIdx] = newPix;
					}
				}
			}
			draw_flag = 1;
			pc += 2;
		} break;

		case 0xE000: {
			// Execute
			if (nn == 0x009E && key[V[x]] > 0)
				pc += 2;
			if (nn == 0x00A1 && key[V[x]] == 0)
				pc += 2;
			pc += 2;
		} break;

		case 0xF000: {
			switch (nn) {
				case 0x0007: {
					// Execute
					V[x] = delay_timer;
					pc += 2;
				} break;

				case 0x000A: {
					// Execute
					bool found = false;

					for (u16 i = 0; i < 16; i++) {
						if (key[i] > 0) {
							V[x] = i;
							found = true;
						}
					}

					if (found)
						pc += 2;
				} break;
				
				case 0x0015: {
					// Execute
					delay_timer = V[x];
					pc += 2;
				} break;

				case 0x0018: {
					// Execute
					sound_timer = V[x];
					pc += 2;
				} break;

				case 0x001E: {
					// Execute
					V[0xF] = I + V[x] > 0xFFF ? 1 : 0;
					I += V[x];
					pc += 2;
				} break;

				case 0x0029: {
					// Execute
					I = V[x] * 0x5;
					pc += 2;
				} break;

				case 0x0033: {
					// Execute
					memory[I + 0] = (V[x] / 100);
					memory[I + 1] = (V[x] / 10) % 10;
					memory[I + 2] = (V[x] % 100) % 10;
					pc += 2;
				} break;

				case 0x0055: {
					// Execute
					for (u16 i = 0; i <= (x); i++) {
						memory[I + i] = V[i];
					}
					I += x + 1;
					pc += 2;
				} break;

				case 0x0065: {
					// Execute
					for (u16 i = 0; i <= (x); i++)
						V[i] = memory[I + i];
					I += x + 1;
					pc += 2;
				} break;
			}
		} break;

		default:
			std::cerr << "Invalid opcode: " << std::hex << opcode << " at: " << pc << std::endl;
	}
}

void Chip8::update_timers() {
	delay_timer -= delay_timer > 0 ? 1 : 0;
	sound_timer -= sound_timer > 0 ? 1 : 0;
}

void Chip8::set_key(int idx, bool enabled) {
	key[idx] = enabled ? 1 : 0;
}

Chip8::Chip8() {}
Chip8::~Chip8() {}