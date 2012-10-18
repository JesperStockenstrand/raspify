/*
 * menu.c
 * 
 * Copyright 2012 Jesper Stockenstrand <jesper@jesper-UX31E>
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


#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#define MainMenu   0
#define Settings   1
#define Playlists  2
#define Playlist   3
#define Playing    4
#define CursorUp   0
#define CursorDown 1


typedef char * string;

struct ListHandler
{
	int ListIndex;
	int CursorPos;
}mmenuLH, settingsLH, playlistLH;

char line1[21] = "";
char line2[21] = "";
char line3[21] = "";
char line4[21] = "";

char playlists[5][30];

int scrollpos=0;
char upperCursor[2] = ">";
char lowerCursor[2] = " ";

int Shuffle = 0;
int Repeat = 0;

int checkButton(int button);
int mygetch();
void updateLCD();
char *scrolled (char *orginaltext, char* desttext, int scrollpos);
char *substring(char *string, int position, int length) ;

int main(int argc, char **argv)
{
	mmenuLH.CursorPos = CursorUp;
	mmenuLH.ListIndex = 0;
	
	settingsLH.CursorPos = CursorUp;
	settingsLH.ListIndex = 0;
	
	playlistLH.CursorPos = CursorUp;
	playlistLH.ListIndex = 0;
	
	strcpy(playlists[0], "Soft ABCDEFGHIJKLMAN");
	strcpy(playlists[1], "Opera");
	strcpy(playlists[2], "the Smiths");
	strcpy(playlists[3], "New Order and other stuff");
	strcpy(playlists[4], "Depeche Mode"); 
	unsigned long arrlen = sizeof(playlists)/sizeof(playlists[0]);
	char texttrunc[15];
	//printf("%s", scrolled(playlists[0], texttrunc, 2));
	
	int button = 0;
	int state = MainMenu;
		
	strcpy(line1, "     MAIN MENU");
	strcpy(line2, "    "); 
	strcat(line2, upperCursor);
	strcat(line2, "Playlists");
	strcpy(line3, "    "); 
	strcat(line3, upperCursor);
	strcat(line3, "Settings");
	strcpy(line4, "     [ ^ ][ v ][SEL]");
		
	while(1)
	{
		switch(state)
		{
			case MainMenu:
				
				strcpy(line1, "     MAIN MENU");
				if (mmenuLH.CursorPos == CursorUp)
				{
					strcpy(upperCursor, ">");
					strcpy(lowerCursor, " ");
				} else
				{
					strcpy(upperCursor, " ");
					strcpy(lowerCursor, ">");
				}	
				strcpy(line2, "    "); 
				strcat(line2, upperCursor);
				strcat(line2, "Playlists");
				strcpy(line3, "    ");
				strcat(line3, lowerCursor);
				strcat(line3, "Settings");
				strcpy(line4, "     [ ^ ][ v ][SEL]");
				updateLCD();
				button = checkButton(button);
				
				switch(button)
				{
					case 2:
						mmenuLH.CursorPos=CursorUp;
						break;
						
					case 3:
						mmenuLH.CursorPos=CursorDown;
						break;
						
					case 4:
						if (mmenuLH.CursorPos==CursorUp)
						{
							state = Playlists;
						} else
						{
							state = Settings;
						}
						break;
						
				}
				break;
				
			case Settings:
				strcpy(line1, "      SETTINGS");
				if (settingsLH.CursorPos == CursorUp)
				{
					strcpy(upperCursor, ">");
					strcpy(lowerCursor, " ");
				} else
				{
					strcpy(upperCursor, " ");
					strcpy(lowerCursor, ">");
				}	
				strcpy(line2, "    "); 
				strcat(line2, upperCursor);
				strcat(line2, "Shuffle");
				if (Shuffle == 1)
				{
					strcat(line2, " ON");
				} else
				{
					strcat(line2, " OFF");
				}
				strcpy(line3, "    ");
				strcat(line3, lowerCursor);
				strcat(line3, "Repeat");
				if (Repeat == 1)
				{
					strcat(line3, " ON");
				} else
				{
					strcat(line3, " OFF");
				}
				strcpy(line4, "[BCK][ ^ ][ v ][SEL]");
				updateLCD();
				button = checkButton(button);
				
				switch(button)
				{
					case 1:
						state = MainMenu;
						break;
						
					case 2:
						settingsLH.CursorPos = CursorUp;
						break;
						
					case 3:
						settingsLH.CursorPos = CursorDown;
						break;
						
					case 4:
						if(settingsLH.CursorPos == CursorUp)
						{
							Shuffle = 1^Shuffle;
						} else
						{
							Repeat = 1^Repeat;
						}
						break;
				}
			break;
			
			case Playlists:
				strcpy(line1, "      PLAYLISTS");
				if (playlistLH.CursorPos == CursorUp)
				{
					strcpy(upperCursor, ">");
					strcpy(lowerCursor, " ");
				} else
				{
					strcpy(upperCursor, " ");
					strcpy(lowerCursor, ">");
				}	
				strcpy(line2, "    "); 
				strcat(line2, upperCursor);
				char *tmpline;
				if (playlistLH.CursorPos == CursorUp)
				{
					
					tmpline = scrolled(playlists[playlistLH.ListIndex], texttrunc, scrollpos++);
					if(scrollpos == strlen(playlists[playlistLH.ListIndex]) + 1)
					{
						scrollpos = -14;
					}
				} else
				{
					tmpline = scrolled(playlists[playlistLH.ListIndex], texttrunc, 0);
				}
				strcat(line2, tmpline);
				strcpy(line3, "    ");
				strcat(line3, lowerCursor);
				//char *tmpline;
				if (playlistLH.CursorPos == CursorDown)
				{
					
					tmpline = scrolled(playlists[playlistLH.ListIndex+1], texttrunc, scrollpos++);
					if(scrollpos == strlen(playlists[playlistLH.ListIndex+1]) + 1)
					{
						scrollpos = -14;
					}
				} else
				{
					tmpline = scrolled(playlists[playlistLH.ListIndex+1], texttrunc, 0);
				}
				strcat(line3, tmpline);
				strcpy(line4, "[BCK][ ^ ][ v ][SEL]");
				updateLCD();
				button = checkButton(button);
				
				switch(button)
				{
					case 1:
						state = MainMenu;
						scrollpos = 0;
						break;
					
					case 2:
						if((playlistLH.CursorPos == CursorUp) && (playlistLH.ListIndex > 0))
						{
							playlistLH.ListIndex--;
						} else
						{
							playlistLH.CursorPos = CursorUp;
						}
						scrollpos = 0;
						break;
						
					case 3:
						if((playlistLH.CursorPos == CursorDown) && (playlistLH.ListIndex < (arrlen-2)))
						{
							playlistLH.ListIndex++;
						} else
						{
							playlistLH.CursorPos = CursorDown;
						}
						scrollpos = 0;
						break;
				}
			
			case Playlist:
			
			break;
			
			case Playing:
			
			break;
		}
		
		usleep(1000);
		
		
		
	}
	
	return 0;
}

char *scrolled (char *orginaltext, char* desttext, int m_scrollpos)
{
	char tmpstr[26];
	strcpy(tmpstr, orginaltext);
	if (strlen(orginaltext) > 15)
	{	
		if (m_scrollpos < 0)
		{
			int i;
			strcpy(tmpstr," ");
			for(i=m_scrollpos; i<-1; i++)
			{
				strcat(tmpstr, " ");
			}
			strcat(tmpstr, orginaltext);
		}
			
		desttext = substring(tmpstr, m_scrollpos, 15);
		
	} else
	{
		strcpy(desttext, orginaltext);
	}
	return desttext;
	
	
}

int checkButton(int button)
{
	int keyPress = mygetch();
	keyPress = keyPress - 48;
	if ((keyPress > 0) || (keyPress < 5))
	{
		return keyPress;
	}
	return 0;
}

int mygetch()
{
  struct termios oldt,
                 newt;
  int            ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}

void updateLCD()
{
	system("clear");
	printf("%s\n", line1);
	printf("%s\n", line2);
	printf("%s\n", line3);
	printf("%s\n", line4);
}

char *substring(char *string, int position, int length) 
{
   char *pointer;
   int c;
 
   pointer = malloc(length+1);
 
   if (pointer == NULL)
   {
      printf("Unable to allocate memory.\n");
      exit(1);
   }
 
   for (c = 0 ; c < position -1 ; c++) 
      string++; 
 
   for (c = 0 ; c < length ; c++)
   {
      *(pointer+c) = *string;      
      string++;   
   }
 
   *(pointer+c) = '\0';
 
   return pointer;
}
