/*
 * menu.c
 * Used for testing menu functionality
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
#include <linux/input.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "lcd.h"

#define MainMenu   0
#define Settings   1
#define Playlists  2
#define Playlist   3
#define Playing    4
#define CursorUp   0
#define CursorDown 1

typedef char * string;

struct ListHandler {
  int ListIndex;
  int CursorPos;
} mmenuLH, settingsLH, playlistLH, tracksLH;

char line1[22] = "";
char line2[22] = "";
char line3[22] = "";
char line4[22] = "";

char playlists[255][255];
char tracks[1024][255];
char playingArtist[255];
char playingTrack[255];
int playingLength = 0;

int SelectedList = 0;
int SelectedTrack = 0;
int nextTrack = 0;
int prevTrack = 0;
int pauseTrack = 0;

int scrollpos=0;
char upperCursor[2] = ">";
char lowerCursor[2] = " ";

int playTimeSec = 0;
time_t menuTimeOut;
int numOfPL;
int numOfTracks;
int playState = 0;

int Shuffle = 0;
int Repeat = 0;

int state;
void updateLCD();
char *scrolled (char *orginaltext, char* desttext, int scrollpos);
char *substring(char *string, int position, int length);
char *tr ( char *s );


void initMenu() {
  //LCD_setup();
  state = MainMenu;
	
  mmenuLH.CursorPos = CursorUp;
  mmenuLH.ListIndex = 0;
	
  settingsLH.CursorPos = CursorUp;
  settingsLH.ListIndex = 0;
	
  playlistLH.CursorPos = CursorUp;
  playlistLH.ListIndex = 0;
	
  tracksLH.CursorPos = CursorUp;
  tracksLH.ListIndex = 0;
}

void updateMenu(int button) {
  char *tmpline;
  int i;
	
  if ((playState == 1) && (difftime(time(NULL), menuTimeOut) > 5)) {
    state = Playing;
  }
	
  switch(state) {
    case MainMenu:
      switch(button) {
        case 2:
          mmenuLH.CursorPos=CursorUp;
          break;
						
        case 3:
          mmenuLH.CursorPos=CursorDown;
          break;
						
        case 4:
          if (mmenuLH.CursorPos==CursorUp) {
            state = Playlists;
            lcd_clear();
            updateMenu(0);
            return;
					} 
          else {
            state = Settings;
            lcd_clear();
            updateMenu(0);
            return;
          }
      }
      strcpy(line1, "     MAIN MENU");
      if (mmenuLH.CursorPos == CursorUp) {
        strcpy(upperCursor, ">");
        strcpy(lowerCursor, " ");
      }
      else {
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
      break;	
			
    case Settings:
      switch(button) {
        case 1:
          state = MainMenu;
          lcd_clear();
          updateMenu(0);
          return;
										
        case 2:
          settingsLH.CursorPos = CursorUp;
          break;
						
        case 3:
          settingsLH.CursorPos = CursorDown;
          break;
						
        case 4:
          if(settingsLH.CursorPos == CursorUp) {
            Shuffle = 1^Shuffle;
          }
          else {
            Repeat = 1^Repeat;
          }
          break;
      }
			
      strcpy(line1, "      SETTINGS");
      if (settingsLH.CursorPos == CursorUp) {
        strcpy(upperCursor, ">");
        strcpy(lowerCursor, " ");
      }
      else {
        strcpy(upperCursor, " ");
        strcpy(lowerCursor, ">");
      }	
      strcpy(line2, "    "); 
      strcat(line2, upperCursor);
      strcat(line2, "Shuffle");
      if (Shuffle == 1) {
        strcat(line2, " ON");
      }
      else {
        strcat(line2, " OFF");
      }
      strcpy(line3, "    ");
      strcat(line3, lowerCursor);
      strcat(line3, "Repeat");
      if (Repeat == 1) {
        strcat(line3, " ON");
      }
      else {
        strcat(line3, " OFF");
      }
      strcpy(line4, "[BCK][ ^ ][ v ][SEL]");
      updateLCD();
      break;
			
      case Playlists:
        switch(button) {
          case 1:
            state = MainMenu;
            lcd_clear();
            updateMenu(0);
            return;
					
          case 2:
            if((playlistLH.CursorPos == CursorUp) && (playlistLH.ListIndex > 0)) {
              playlistLH.ListIndex--;
            }
            else {
              playlistLH.CursorPos = CursorUp;
            }
            SelectedList = playlistLH.ListIndex + playlistLH.CursorPos + 1;
            break;
						
          case 3:
            if((playlistLH.CursorPos == CursorDown) && (playlistLH.ListIndex < numOfPL-2)) {
              playlistLH.ListIndex++;
            } 
            else {
              playlistLH.CursorPos = CursorDown;
            }
            SelectedList = playlistLH.ListIndex + playlistLH.CursorPos + 1;
            break; 
						
          case 4:
            state = Playlist;
            tracksLH.CursorPos = CursorUp;
            tracksLH.ListIndex = 0;
            lcd_clear();
            updateMenu(0);
            return;
        }
        strcpy(line1, "      PLAYLISTS");
        if (playlistLH.CursorPos == CursorUp) {
          strcpy(upperCursor, ">");
          strcpy(lowerCursor, " ");
        } 
        else {
          strcpy(upperCursor, " ");
          strcpy(lowerCursor, ">");
        }	
        strcpy(line2, upperCursor);
				
        tmpline = substring(playlists[playlistLH.ListIndex],0,19);
        strcat(line2, tmpline);
        strcpy(line3, lowerCursor);
        tmpline = substring(playlists[playlistLH.ListIndex+1],0,19);
        strcat(line3, tmpline);
        strcpy(line4, "[BCK][ ^ ][ v ][SEL]");
        updateLCD();
        break;
				
      case Playlist:
        switch(button) {
          case 1:
            state = Playlists;
            lcd_clear();
            updateMenu(0);
            return;
					
          case 2:
            if((tracksLH.CursorPos == CursorUp) && (tracksLH.ListIndex > 0)) {
              tracksLH.ListIndex--;
            }
            else {
              tracksLH.CursorPos = CursorUp;
            }
            break;
						
          case 3:
            if((tracksLH.CursorPos == CursorDown) && (tracksLH.ListIndex < numOfTracks-2)) {
              tracksLH.ListIndex++;
            } 
            else {
              tracksLH.CursorPos = CursorDown;
            }
            break; 
						
          case 4:
            SelectedTrack = tracksLH.ListIndex + tracksLH.CursorPos + 1;
            state = Playing;
            lcd_clear();
            updateMenu(0);
            return; 
        }
        strcpy(line1, "      TRACKS");
        if (tracksLH.CursorPos == CursorUp) {
          strcpy(upperCursor, ">");
          strcpy(lowerCursor, " ");
        } 
        else {
          strcpy(upperCursor, " ");
          strcpy(lowerCursor, ">");
        }	
        strcpy(line2, upperCursor);
        tmpline = substring(tracks[tracksLH.ListIndex],0,19);
        strcat(line2, tmpline);
        strcpy(line3, lowerCursor);
        tmpline = substring(tracks[tracksLH.ListIndex+1],0,19);
        strcat(line3, tmpline);
        strcpy(line4, "[BCK][ ^ ][ v ][SEL]");
				updateLCD();
        break;
			
      case Playing:
        switch(button) {
          case 1:
            state = Playlist;
            lcd_clear();
            updateMenu(0);
            return;
					
          case 2:
            prevTrack = 1;
            break;
						
          case 3:
            pauseTrack = 1^pauseTrack;
            break; 
						
          case 4:
            nextTrack = 1;	
            break;
					
        }
        tmpline = tr(substring(playingArtist,0,20));
        strcpy(line1,"");
        
        if (strlen(tmpline)<20) {
          for (i=0;i<((20-strlen(tmpline))/2);i++) {
            strcat(line1," ");
          }
        }
        strcat(line1, tmpline);
        
        tmpline = tr(substring(playingTrack,0,20));
        strcpy(line2,"");
        
        if (strlen(tmpline)<20) {
          for (i=0;i<((20-strlen(tmpline))/2);i++) {
            strcat(line2," ");
          }
        }
        strcat(line2, tmpline);
        
        sprintf(tmpline, "      %d:%02d/%d:%02d", playTimeSec / 60, playTimeSec % 60, playingLength / 60, playingLength % 60);
        strcpy(line3, tmpline);
        strcpy(line4, "[MNU][|<<][ ||][>>|]");
        updateLCD();
        break;
    }
}

char *scrolled (char *orginaltext, char* desttext, int m_scrollpos) {
  char tmpstr[26];
  strcpy(tmpstr, orginaltext);
  if (strlen(orginaltext) > 15) {	
    if (m_scrollpos < 0) {
      int i;
      strcpy(tmpstr," ");
      for(i=m_scrollpos; i<-1; i++) {
        strcat(tmpstr, " ");
      }
      strcat(tmpstr, orginaltext);
    }
    desttext = substring(tmpstr, m_scrollpos, 15);
	}
  else {
    strcpy(desttext, orginaltext);
  }
  return desttext;
}

void updateLCD() {
  system("clear");
  printf("%s\n", line1);
  printf("%s\n", line2);
  printf("%s\n", line3);
  printf("%s\n", line4);
	//lcd_clear();
	//write_nibbles(CMD_CAH);
  lcd_line(line1);
  lcd_line(line3);
  lcd_line(line2);
  lcd_line(line4);
}

char *substring(char *string, int position, int length) {
  char *pointer;
  int c;
 
  pointer = malloc(length+2);
 
  if (pointer == NULL) {
    strcpy(line2, "Unable to allocate mem");
    updateLCD();
    exit(1);
  }
 
  for (c = 0 ; c < position -1 ; c++) { string++; } 
  for (c = 0 ; c < length ; c++) {
    *(pointer+c) = *string;      
    string++;   
  }
 
  *(pointer+c) = '\0';
 
  return pointer;
}

char *tr ( char *s )
{
  int i = 0;
  int j = strlen ( s ) - 1;
  int k = 0;
 
  while ( isspace ( s[i] ) && s[i] != '\0' )
    i++;
 
  while ( isspace ( s[j] ) && j >= 0 )
    j--;
 
  while ( i <= j )
    s[k++] = s[i++];
 
  s[k] = '\0';
 
  return s;
}
