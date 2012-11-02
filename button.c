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


int is_key_pressed(int fd, int key);

int preButton = 0;

int checkButton() {
	int fd;
	fd = open ("/dev/input/event2", O_RDONLY);  //event2 on work computer, event3 at home
	if (is_key_pressed(fd, KEY_1) == 1) {
		if (preButton != 1)	{
			preButton = 1;
			return 1;
		}
	} else if (is_key_pressed(fd, KEY_2) == 1) {
		if (preButton != 2) {
			preButton = 2;
			return 2;
		}
	} else if (is_key_pressed(fd, KEY_3) == 1) {
		if (preButton != 3) {
			preButton = 3;
			return 3;
		}
	} else if (is_key_pressed(fd, KEY_4) == 1) {
		if (preButton != 4) {
			preButton = 4;
			return 4;
		}
	} else {
		preButton = 0;
		return 0;
	}
	return 0;
}

int is_key_pressed(int fd, int key) {
    char key_b[(KEY_MAX + 7) / 8];

    memset(key_b, 0, sizeof(key_b));
    ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b);
        
    return !!(key_b[key/8] & (1<<(key % 8)));
}
