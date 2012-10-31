// lcd.c by j3557o
//          __________
//         |    U     | 
//    gnd -|A0     Vdd|- 3v3 from RPi
//    gnd -|A1     SDA|- SDL from RPi
//    gnd -|A2     SCL|- SCL from RPi
//   DB04 -|P0 i2c INT|-
//   DB05 -|P1      P7|- RS
//   DB06 -|P2      P6|-
//   DB07 -|P3      P5|- EN
//    gnd -|Vss     P4|-
//         |__________|
  

#include <stdio.h>
#include <linux/i2c-dev.h>

//Define pin nr for EN and RS
#define LCD_RS 0x80
#define LCD_EN 0x20

//Define LCD settings
#define CMD_SCMD 0x04	//Set Cursor Move Direction:
#define SCMD_ID 0x02	//- Increment the Cursor After Each Byte Written to Display if Set
#define SCMD_S 0x01		//- Shift Display when Byte Written to Display

#define CMD_EDC 0x08	//Enable Display/Cursor
#define EDC_D 0x04 		//- Turn Display On
#define EDC_C 0x02 		//- Turn Cursor On
#define EDC_B 0x01 		//- Cursor Blink On

#define CMD_MCSD 0x10	//Move Cursor/Shift Display
#define MCSD_SC 0x08 	//- Display Shift On(1)/Off(0)
#define MCSD_RL 0x04 	//- Direction of Shift Right(1)/Left(0)

#define CMD_SIL 0x20 	//Set Interface Length
#define SIL_DL 0x10 	//- Set Data Interface Length 8
#define SIL_N 0x08 		//- Number of Display Lines 2(=4)
#define SIL_F 0x04 		//- Character Font 5x10

#define CMD_MCD 0x80 	//Move Cursor to Display Address
#define CMD_CAH	 0x01	//Clear and Home

int fd;
char *fileName = "/dev/i2c-0";
int address = 0x20;

void lcd_reset();
void lcd_init();
void PutBitsOnPins(int bits);
void write_nibbles(int bits);
void write_lcd(int bits);

void LCD_setup() {
	if ((fd = open(fileName, O_RDWR)) < 0) {
		printf("Failed to open the i2c bus");
		exit(1);
	}

	if (ioctl(fd,I2C_SLAVE,address) < 0) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		exit(1);
	}

	lcd_reset();
	lcd_init();
}

void write_lcd(int bits) {
	PutBitsOnPins(bits|LCD_EN);
	usleep(5000);
	PutBitsOnPins(bits);
	usleep(5000);
	PutBitsOnPins(0x00);
	usleep(5000);
}

void PutBitsOnPins(int bits) {
	char buf[1];
	buf[0] = bits;
	if(write(fd, buf, 1) != 1) {
		printf("Failed to write to the i2c bus.\n");
	}
}

void lcd_reset() {
	write_lcd(0x03);
	usleep(5000);
	write_lcd(0x03);
	usleep(5000);
	write_lcd(0x03);
	usleep(5000);
	write_lcd(0x02);
	usleep(5000);
}

void lcd_init() {
	write_nibbles(CMD_SIL|SIL_N);
	write_nibbles(CMD_EDC);
	write_nibbles(CMD_CAH);
	write_nibbles(CMD_SCMD|SCMD_ID);
	write_nibbles(CMD_EDC|EDC_D);
}

void write_nibbles(int bits) {
	write_lcd((bits >> 4) & 0x0F);
	write_lcd(bits & 0x0F);
}

void write_char(char letter) {
	write_lcd((((int)letter >> 4) & 0x0F)|LCD_RS);
	write_lcd(((int)letter & 0x0F)|LCD_RS);
}
