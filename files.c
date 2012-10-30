/*
 * files.c
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


#include <dirent.h>  
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>

int main(void) {
	/*DIR *dir;
	struct dirent *ent;
	char folders[256][256];
    dir = opendir ("music");
    int i = 0;
	if (dir != NULL) {
		
		// print all the files and directories within directory 
		while ((ent = readdir(dir)) != NULL) {
			if((strcmp(ent->d_name, ".")) && (strcmp(ent->d_name, ".."))) {
				strcpy(folders[i], ent->d_name); 
				i++;
			}
		}
		closedir (dir);
	} else {
		 could not open directory 
		perror ("");
		return EXIT_FAILURE;
	}
	int j;
	for(j = 0; j < i;j++) {
		printf("%s\n",folders[j]);
	}*/
	char getfolders[256][256]; 
	getfolders = folderlist("/home/jesper");
	
	return 0;
}

char folderlist[256][256] (DIR *path) {
	struct dirent *ent;
	dir = opendir ("music");
    int i = 0;
    char folders[256][256];
	if (dir != NULL) {
		
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if((strcmp(ent->d_name, ".")) && (strcmp(ent->d_name, ".."))) {
				strcpy(folders[i], ent->d_name); 
				i++;
			}
		}
		closedir (dir);
	}
	return folders; 
}
