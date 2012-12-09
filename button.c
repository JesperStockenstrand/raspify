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
int fd;
  
char *BUTTONS = "/dev/i2c-0";
int BTNaddress = 0x24;
int buttonsConnected = -1; // -1 = unknown, 0 = not connected, 1 = connected

int preButton = 0;


void initButtons() {
  fd = open("/dev/input/event3", O_RDONLY);  //event2 on work computer, event3 at home
	
  if ((btn = open(BUTTONS, O_RDWR)) < 0) {
    printf("Failed to open the i2c bus\n");
    buttonsConnected = 0;
    return;
  }

  if (ioctl(btn, I2C_SLAVE, BTNaddress) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
    buttonsConnected = 0;
    return;
  }
  buttonsConnected = 1;
}

int checkButton() {
  char buf[1];
  int button = 0;
  
  if (buttonsConnected == -1) {
    initButtons();
  }
  
  
	if (is_key_pressed(fd, KEY_1) == 1) {
		if (preButton != 1)	{
			preButton = 1;
			button = 1;
    }
	} else if (is_key_pressed(fd, KEY_2) == 1) {
		if (preButton != 2) {
			preButton = 2;
			button = 2;
		}
	} else if (is_key_pressed(fd, KEY_3) == 1) {
		if (preButton != 3) {
			preButton = 3;
			button = 3;
		}
	} else if (is_key_pressed(fd, KEY_4) == 1) {
		if (preButton != 4) {
			preButton = 4;
			button = 4;
		}
	}
  else {
    preButton = 0;
    button = 0;
  }
  
  if (buttonsConnected == 1) {
    
  
  
    if (read(btn, buf, 1) != 1) {
      printf("Error reading from i2c\n");
      buttonsConnected = 0;
    }
    else {
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
  }
  return button;
  
}

int is_key_pressed(int fd, int key) {
  char key_b[(KEY_MAX + 7) / 8];

  memset(key_b, 0, sizeof(key_b));
  ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b);
        
  return !!(key_b[key/8] & (1<<(key % 8)));
}
