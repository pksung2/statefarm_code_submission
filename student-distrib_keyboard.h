#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "terminal.h"

/* Keyboard-Input Related */
#define kb_irq_num 			1
#define KB_DATA_PORT		0x60
#define KEY_RELEASE_CHECK 	0x80

/* General Scancode Related */
#define LSHIFT_PRESS		0x2A
#define RSHIFT_PRESS		0x36
#define LSHIFT_RELEASE		0xAA
#define RSHIFT_RELEASE		0xB6
#define CAPS_PRESS			0x3A
#define ENTER_PRESS			0x1C
#define SPACE_PRESS			0x39
#define TAB_PRESS			0x0F
#define BACKSPACE_PRESS		0x0E
#define DELETE_PRESS		0x53
#define CRTL_PRESS			0x1D
#define CRTL_RELEASE		0x9D

#define DEFAULT			0
#define SHIFT			1
#define CAPS 			2
#define SHIFT_CAPS		3
#define NUM_SCANCODES	60
#define TAB_SIZE		4

/* Terminal Related */
#define ALT_PRESS			0x38
#define ALT_RELEASE			0xB8
#define F1_PRESS			0x3B
#define F2_PRESS			0x3C
#define F3_PRESS			0x3D
#define F4_PRESS			0x3E
#define F5_PRESS			0x3F
#define F6_PRESS			0x40

#define F1 					0
#define F2 					1
#define F3 					2
#define F4 					3
#define F5 					4
#define F6 					5

/* ATTRIB Related */
#define ONE					2
#define ZERO				0x0B
#define WHITE				7
#define TEAL 				3
#define ZEST				12
#define RED 				4

/* Arrow Related */
#define LEFT_PRESS			0x4B
#define UP_PRESS			0x48
#define RIGHT_PRESS			0x4D
#define DOWN_PRESS			0x50

#define LEFT				0
#define UP					1
#define RIGHT				2
#define DOWN				3

/* Annoyed */
#define NOT_ANNOYED			5
#define KINDA_ANNOYED		10
#define REALLY_ANNOYED		20
#define DEATH				27
#define DEATH_X				NUM_COLS*3/8

extern char input_buf[BUF_SIZE];
extern int input_index;
extern int input_size;

//functions that initialize and handle the keyboard interrupt
void keyboard_init();
void keyboard_handler();
void handle_scancode(int scancode);
void handle_enter(int sameline);
void handle_backspace();
void handle_delete();
void handle_arrows(int direction);
void restore_prev_input(int idx);
void restore_next_input(int idx);
void max_terminal(int terminal_number);

#endif
