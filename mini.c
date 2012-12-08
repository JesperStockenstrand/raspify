/* mini
 * A minimal despotify client, based on simple client.
 * 
 * 
 *
 */

#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "despotify.h"
#include "util.h"
#include "button.c"
#include "menu.c"

static void* audio_device;
char *wrapper_read_command(void);
struct track* new_t;
char track_id[33];

/****** PCM thread stuff: ***********/
static pthread_t thread;
static pthread_mutex_t thread_mutex;
static pthread_cond_t thread_cond;
static enum {
  PAUSE,
  PLAY,
  EXIT
} play_state = PAUSE;

static void thread_play(void) {
  pthread_mutex_lock(&thread_mutex);
  play_state = PLAY;
  pthread_cond_signal(&thread_cond);
  pthread_mutex_unlock(&thread_mutex);
}

static void thread_pause(void) {
  pthread_mutex_lock(&thread_mutex);
  play_state = PAUSE;
  pthread_mutex_unlock(&thread_mutex);
}

static void thread_exit(void) {
  pthread_mutex_lock(&thread_mutex);
  play_state = EXIT;
  pthread_cond_signal(&thread_cond);
  pthread_mutex_unlock(&thread_mutex);

  pthread_join(thread, NULL);
}

static void* thread_loop(void* arg) {
  struct despotify_session* ds = arg;
  struct pcm_data pcm;
    
  pthread_mutex_init(&thread_mutex, NULL);
  pthread_cond_init(&thread_cond, NULL);

  bool loop = true;
  while (loop) {
    switch (play_state) {
      case PAUSE:
        pthread_mutex_lock(&thread_mutex);
        pthread_cond_wait(&thread_cond, &thread_mutex);
        pthread_mutex_unlock(&thread_mutex);
        break;

      case PLAY: {
        int rc = despotify_get_pcm(ds, &pcm);
        if (rc == 0) {
          audio_play_pcm(audio_device, &pcm);
        }
        else {
          strcpy(line2, "get_pcm error");
          updateLCD();
        }
        break;
      }

      case EXIT:
        loop = false;
        break;
    }
  }

  pthread_cond_destroy(&thread_cond);
  pthread_mutex_destroy(&thread_mutex);

  return NULL;
}

/**************** UI (main) thread stuff: ***************/

struct playlist* get_playlist(struct playlist* rootlist, int num) {
  struct playlist* p = rootlist;

	/* skip to playlist number <num> */
  for (int i = 1; i < num && p; i++) {
    p = p->next;
  }

  if (!p) {
    strcpy(line2, "Invalid pl number");
    updateLCD();
  }
  return p;
}

//PRINT PLAYLISTS
void print_list_of_lists(struct playlist* rootlist) {
  if (!rootlist) {
    strcpy(line2, "<no stored playlists>");
    updateLCD();
  }
  else {
    int count=0;
    //PRINT ALL PLAYLISTS
    for (struct playlist* p = rootlist; p; p = p->next) {
      strcpy(playlists[count++], p->name);
    }
    numOfPL = count;
  }
}

//PRINT TRACKS IN PLAYLIST
void print_tracks(struct track* head) {
  if (!head) {
    strcpy(line2, "<empty playlist>");
    updateLCD();
    return;
  }

  int count = 0;
  for (struct track* t = head; t; t = t->next) {
    if (t->playable) {
      if (t->has_meta_data) {
        strcpy(tracks[count], t->title);
        strcat(tracks[count], " - ");
        for (struct artist* a = t->artist; a; a = a->next) {
          strcat(tracks[count], a->name);
          strcat(tracks[count], " ");
        }
      }
      else {
        strcpy(tracks[count], "N/A");
      }
    }
    else {
      strcpy(tracks[count],"UNPLAYABLE");
    }
    count++;
  }
  numOfTracks = count;
}

//THIS IS THE MAIN PROGRAM LOOP
void command_loop(struct despotify_session* ds) {
  bool loop = true;
  char *buf;
  struct playlist* rootlist = NULL;
  struct playlist* lastlist = NULL;
	
  int preSelectedList = 1;
  int preSelectedTrack = 0;
  int pauseMode = 0;
	
  int buttonpressed = 0;

  if (!rootlist) {
    rootlist = despotify_get_stored_playlists(ds);
  }
  print_list_of_lists(rootlist);
		
  do {
    buttonpressed = checkButton();
		
    // BUTTON HAS BEEN PRESSED, UPDATE GUI
    if (buttonpressed != 0) { 
      menuTimeOut = time(NULL);
      updateMenu(buttonpressed); 
    }
    else {
      updateMenu(0);
    }
		// NEW PLAYLIST MARKED IN GUI
    if (SelectedList != preSelectedList) {
      struct playlist* p = get_playlist(rootlist, SelectedList);
      if (p) {
        print_tracks(p->tracks);
        lastlist = p;
        preSelectedList = SelectedList;
      }
      preSelectedTrack = 0;
    }
			
		// NEW TRACK SELECTED IN GUI
    if (SelectedTrack != preSelectedTrack) {
      struct track* t;
      t = lastlist->tracks;
      for (int i=1; i<SelectedTrack && t; i++) {
        t = t->next;
      }
      if (t) {
        preSelectedTrack = SelectedTrack;
        thread_pause();
        despotify_stop(ds);
        despotify_play(ds, t, true);
        thread_play();
        playState = 1;
      }
    }
			
		// PLAY NEXT TRACK
    if (nextTrack == 1) {
      despotify_next(ds);
      playState = 1;
      nextTrack = 0;
    }
			
		// PLAY PREVIOUS TRACK (OR START FROM BEGINNING OF TRACK IF
		// FIRST TRACK OR IF TRACK HAS BEEN RUNNING FOR MORE THAN 3 SECONDS
    if (prevTrack == 1) {
      struct track* t;
      int i;
      t = lastlist->tracks;
      for (i=1; (strcmp((char*)t->track_id, track_id) != 0) && t; i++) {
        t = t->next;
      }
      if ((playTimeSec > 3) || (i == 1)) {
        thread_pause();
        despotify_stop(ds);
        despotify_play(ds, t, true);
        thread_play();
      } 
      else {			
        t = lastlist->tracks;
        for (int i=1; i<SelectedTrack && t; i++) {
          t = t->next;
        }
        if (t) {
          preSelectedTrack = SelectedTrack;
          thread_pause();
          despotify_stop(ds);
          despotify_play(ds, t, true);
          thread_play();
        }
      }
      playState = 1;
      prevTrack = 0;
    }
       
    // pause 
    if (pauseMode != pauseTrack) {
      if (pauseTrack == 1) {
        thread_pause();
        playState = 0;
      } 
      else {
        thread_play();
      }
      pauseMode = pauseTrack;
    }
		
		/*
		//	resume 
        else if (!strncmp(buf, "resume", 5)) {
            thread_play();
        }

        */
    usleep(2000);
  } while(loop);

  if (rootlist) { 
    despotify_free_playlist(rootlist); 
  }
}

void callback(struct despotify_session* ds, int signal, void* data, void* callback_data) {
  static int seconds = -1;
  (void)ds; 
  (void)callback_data; /* don't warn about unused parameters */

  switch (signal) {
    case DESPOTIFY_NEW_TRACK: {
      struct track* t = data;
            
      strcpy(playingArtist, t->artist->name);
      strcpy(playingTrack, t->title);
      playingLength = t->length/1000;
      strcpy(track_id, (char*)t->track_id);
      updateMenu(0);
      break;
    }
        
    case DESPOTIFY_TIME_TELL:
      if ((int)(*((double*)data)) != seconds) {
        seconds = *((double*)data);
        playTimeSec = seconds;
      }
      break;

    case DESPOTIFY_END_OF_PLAYLIST: 
      if (Repeat != 1) {
        thread_pause();
        playState = 0;
        updateMenu(1);
      } 
      else {
        SelectedTrack = 1;
      }
      break;
		
  }
}

int main(int argc, char** argv) {
  //STARTING UP
  initMenu();
	
  strcpy(line2, "Starting Raspify");
  updateLCD();
  setlocale(LC_ALL, "");
	
	//CHECK ARGS
  if (argc < 3) {
    printf("Usage: %s <username> <password>\n", argv[0]);
    return 1;
  }
    
  //DESPOTIFY INIT
  if (!despotify_init()) {
    strcpy(line2, "init failed");
    updateLCD();
    return 1;
  }
	
	//CREATE DESPOTIFY SESSION
  struct despotify_session* ds = despotify_init_client(callback, NULL, true, true);
  if (!ds) {
    strcpy(line2, "client failed");
    updateLCD();
    return 1;
  }
	
	//START/TRANSFER SESSION TO SEPARATE THREAD
  pthread_create(&thread, NULL, &thread_loop, ds);
	
	//TRY SPOTIFY LOGIN
  if (!despotify_authenticate(ds, argv[1], argv[2])) {
    strcpy(line2, "Auth failed");
    updateLCD();
    despotify_exit(ds);
    return 1;
  }
  strcpy(line2, "Logged in to Spotify");
  updateLCD();
    
  audio_device = audio_init();
  initButtons();
  updateMenu(0);
	
  //START MAIN LOOP
  command_loop(ds);
    
  //START GRACEFUL SHUTDOWN
  thread_exit();
  audio_exit(audio_device);
  despotify_exit(ds);
    
  if (!despotify_cleanup()) {
    return 1;
  }

  return 0;
}

