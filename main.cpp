#ifndef MAIN_H
#define MAIN_H

#include "src/chip8.h"
#include "src/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "glut.h"

#define SCALE 4

Chip8 chip8;
f32 rate = 6000.0f;
f64 prev_time;
f64 prev_timer_time;
f64 curr_time;
u08 viewport_texture[64*32*3];
u32 viewport_width = 64 * SCALE;
u32 viewport_height = 32 * SCALE;

u32 on_color, off_color;

int main(int argc, char **argv);
bool load_program(char *filename);
void init_gl(int argc, char **argv);
void update_texture();

void window_idle();
void window_reshape(GLsizei w, GLsizei h);
void window_key_down(unsigned char key, int x, int y);
void window_key_up(unsigned char key, int x, int y);

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Usage: chip8.exe [frequency] [program filename]\n");
		return 1;
	}

	rate = (f32)strtod(argv[1], nullptr);

	if (!load_program(argv[2]))
		return 1;
	
	printf("Frequency rate: %.2f Hz\n", rate);
	printf("Press ESC to quit\n");

	curr_time = (f32)clock();
	prev_time = curr_time;
	prev_timer_time = curr_time;

	on_color = (0xFF << 16) | (0xFF << 8) | (0x40 << 0);
	off_color = (0x64 << 16) | (0x32 << 8) | (0x00 << 0);

	init_gl(argc, argv);

	glutMainLoop();

	return 0;
}

bool load_program(char *filename) {
	printf("Loading: %s\n", filename);
	FILE *file = fopen(filename, "rb");
	if (file == nullptr) {
		fputs("Error when loading program", stderr); 
		return false;
	}

	fseek(file , 0 , SEEK_END);
	long file_size = ftell(file);
	rewind(file);
	printf("Filesize: %d bytes\n", (int)file_size);

	if (file_size > 0x0E00) {
		fputs("Program too big", stderr); 
		return false;
	}

	char *buffer = (char*)malloc(sizeof(char) * file_size);
	if (buffer == NULL) {
		fputs("Out of memory error when loading program", stderr); 
		return false;
	}

	size_t result = fread (buffer, 1, file_size, file);
	if (result != file_size) {
		fputs("Error when reading program", stderr); 
		return false;
	}

	int err = chip8.load_program((u08 *)buffer, (u16)file_size);

	fclose(file);
	free(buffer);

	if (err != ERR_OK) {
		fputs("Error when loading program", stderr);
		return false;
	}

	return true;
}

void init_gl(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(viewport_width, viewport_height);
	glutInitWindowPosition(320, 320);
	glutCreateWindow("Chip8 emulator");

	glutDisplayFunc(window_idle);
	glutIdleFunc(window_idle);
    glutReshapeFunc(window_reshape);        
	glutKeyboardFunc(window_key_down);
	glutKeyboardUpFunc(window_key_up);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 64, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, (void *)viewport_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 

	glEnable(GL_TEXTURE_2D);
}

void update_texture() {
	u08 *display = chip8.get_display();
	u08 on_r = (on_color >> 16) & 0xFF;
	u08 on_g = (on_color >> 8) & 0xFF;
	u08 on_b = (on_color >> 0) & 0xFF;
	u08 off_r = (off_color >> 16) & 0xFF;
	u08 off_g = (off_color >> 8) & 0xFF;
	u08 off_b = (off_color >> 0) & 0xFF;
	for (int j = 0; j < 32; j++) {
		for (int i = 0; i < 64; i++) {
			if (display[j * 64 + i] > 0) {
				viewport_texture[(j * 64 + i) * 3 + 0] = on_r;
				viewport_texture[(j * 64 + i) * 3 + 1] = on_g;
				viewport_texture[(j * 64 + i) * 3 + 2] = on_b;
			} else {
				viewport_texture[(j * 64 + i) * 3 + 0] = off_r;
				viewport_texture[(j * 64 + i) * 3 + 1] = off_g;
				viewport_texture[(j * 64 + i) * 3 + 2] = off_b;
			}
		}
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGB, GL_UNSIGNED_BYTE, (void *)viewport_texture);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0); glVertex2d(0.0, 0.0);
	glTexCoord2d(1.0, 0.0); glVertex2d(viewport_width, 0.0);
	glTexCoord2d(1.0, 1.0); glVertex2d(viewport_width, viewport_height);
	glTexCoord2d(0.0, 1.0); glVertex2d(0.0, viewport_height);
	glEnd();
}

void window_idle() {
	curr_time = (f32)clock();
	bool draw = false;

	while (curr_time - prev_time > 1000 / rate) {
		chip8.cycle();

		draw |= chip8.should_draw();

		prev_time += 1000 / rate;
	}

	while (curr_time - prev_timer_time > 1000 / 60.0f) {
		chip8.update_timers();

		prev_timer_time += 1000 / 60.0f;
	}

	if (draw) {
		glClear(GL_COLOR_BUFFER_BIT);

		update_texture();

		glutSwapBuffers();
	}
}

void window_reshape(GLsizei w, GLsizei h) {
	glClearColor(0, 0, 0.5f, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);

	viewport_width = w;
	viewport_height = h;
}

void window_key_down(unsigned char key, int x, int y) {
	if (key == 27)
		exit(0);
	
	switch (key) {
		case '1': chip8.set_key(0x1, true); break;
		case '2': chip8.set_key(0x2, true); break;
		case '3': chip8.set_key(0x3, true); break;
		case '4': chip8.set_key(0xC, true); break;

		case 'q': chip8.set_key(0x4, true); break;
		case 'w': chip8.set_key(0x5, true); break;
		case 'e': chip8.set_key(0x6, true); break;
		case 'r': chip8.set_key(0xD, true); break;

		case 'a': chip8.set_key(0x7, true); break;
		case 's': chip8.set_key(0x8, true); break;
		case 'd': chip8.set_key(0x9, true); break;
		case 'f': chip8.set_key(0xE, true); break;

		case 'z': chip8.set_key(0xA, true); break;
		case 'x': chip8.set_key(0x0, true); break;
		case 'c': chip8.set_key(0xB, true); break;
		case 'v': chip8.set_key(0xF, true); break;
	}
}

void window_key_up(unsigned char key, int x, int y) {
	if (key == 27)
		exit(0);
	
	switch (key) {
		case '1': chip8.set_key(0x1, false); break;
		case '2': chip8.set_key(0x2, false); break;
		case '3': chip8.set_key(0x3, false); break;
		case '4': chip8.set_key(0xC, false); break;

		case 'q': chip8.set_key(0x4, false); break;
		case 'w': chip8.set_key(0x5, false); break;
		case 'e': chip8.set_key(0x6, false); break;
		case 'r': chip8.set_key(0xD, false); break;

		case 'a': chip8.set_key(0x7, false); break;
		case 's': chip8.set_key(0x8, false); break;
		case 'd': chip8.set_key(0x9, false); break;
		case 'f': chip8.set_key(0xE, false); break;

		case 'z': chip8.set_key(0xA, false); break;
		case 'x': chip8.set_key(0x0, false); break;
		case 'c': chip8.set_key(0xB, false); break;
		case 'v': chip8.set_key(0xF, false); break;
	}
}

#endif