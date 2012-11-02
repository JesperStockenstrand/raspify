/*
 * button.c
 * 
 * Copyright 2012 Jesper <jesper@Nemo>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

//Used to check if keys 1, 2, 3 or 4 are pressed on the keyboard.
//Currently hard coded to a device driver
//Will be extended to handle buttons connected through I2C


#include <linux/input.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

int btn;
int kbd;
char *LCD = "/dev/i2c-0";
char *BUTTONS = "/dev/i2c-0";
int LCDaddress = 0x20;
int BTNaddress = 0x24;

int is_key_pressed(int fd, int key);

int preButton = 0;
int buttonsConnected = 0;


void initButtons() {
	if ((btn = open(BUTTONS, O_RDWR)) < 0) {
		printf("Failed to open the i2c bus");
		return;
	}

	if (ioctl(btn, I2C_SLAVE, BTNaddress) < 0) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		return;
	}
	
	kbd = open ("/dev/input/event3", O_RDONLY);  //event2 on work computer, event3 at home
	
	buttonsConnected = 1;
	
}

int checkButton() {
	char buf[1];
	int button = 0;
	
	
	
	if (is_key_pressed(kbd, KEY_1) == 1) {
		if (preButton != 1)	{
			preButton = 1;
			button = 1;
		}
	} else if (is_key_pressed(kbd, KEY_2) == 1) {
		if (preButton != 2) {
			preButton = 2;
			button = 2;
		}
	} else if (is_key_pressed(kbd, KEY_3) == 1) {
		if (preButton != 3) {
			preButton = 3;
			button = 3;
		}
	} else if (is_key_pressed(kbd, KEY_4) == 1) {
		if (preButton != 4) {
			preButton = 4;
			button = 4;
		}
	} else {
		preButton = 0;
		//button = 0;
	}
	
	if (read(btn, buf, 1) != 1) {
		printf("Error reading from i2c");
	} else {
		
		switch(buf[0]) {
			case 13:
				printf("8");
				break;
				
			case 7:
				printf("7");
				break;
				
			case 3:
				printf("6");
				break;
				
			case 1:
				printf("5");
				break;
		}
	}
	return button;
}

int is_key_pressed(int fd, int key) {
    char key_b[(KEY_MAX + 7) / 8];

    memset(key_b, 0, sizeof(key_b));
    ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b);
        
    return !!(key_b[key/8] & (1<<(key % 8)));
}
