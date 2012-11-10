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
char *BUTTONS = "/dev/i2c-0";
int BTNaddress = 0x24;


int preButton = 0;


void initButtons() {
	if ((btn = open(BUTTONS, O_RDWR)) < 0) {
		printf("Failed to open the i2c bus\n");
		return;
	}

	if (ioctl(btn, I2C_SLAVE, BTNaddress) < 0) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		return;
	}
	
}

int checkButton() {
	char buf[1];
	int button = 0;
	if (read(btn, buf, 1) != 1) {
		printf("Error reading from i2c\n");
	} else {
		
		switch(buf[0]) {
			case 127:
				if (preButton != 1) {
					preButton = 1;
					button = 1;
				}
				break;
				
			case 191:
				if (preButton != 2) {
					preButton = 2;
					button = 2;
				}
				break;
				
			case 223:
				if (preButton != 3) {
					preButton = 3;
					button = 3;
				}
				break;
				
			case 239:
				if (preButton != 4) {
					preButton = 4;
					button = 4;
				}
				break;
			
			default:
				preButton = 0;

		}
	}
	return button;
}

