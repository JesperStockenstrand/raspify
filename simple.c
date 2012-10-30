/*
 * A minimal despotify client, based on simple client.
 *
 * $Id: simple.c 514 2010-12-13 22:15:51Z dstien $
 *
 */

#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <wchar.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "audio.h"
#include "despotify.h"
#include "util.h"



static void* audio_device;
char *wrapper_read_command(void);


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
                    printf("despotify_get_pcm() returned error %d\n", rc);
                    exit(-1);
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
            printf("Invalid playlist number %d\n", num);
	}


    return p;
}


//GET PLAYLISTS
void print_list_of_lists(struct playlist* rootlist) {
// Step forward to index start point
// Read two list at the time
// for (int i = 1; i < num && p; i++) {
//            p = p->next;
//	}
// line2 = p->name;
// p->next;
// line3 = p->name;

    if (!rootlist) {
        printf(" <no stored playlists>\n");
    }
    else {
        int count=1;
        //PRINT ALL PLAYLISTS
        for (struct playlist* p = rootlist; p; p = p->next) {
            printf("%2d: %-40s\n", count++, p->name);       
	}
    }
}

//PRINT TRACKS IN PLAYLIST
void print_tracks(struct track* head) {
    if (!head) {
        printf(" <empty playlist>\n");
        return;
    }

    int count = 1;
    for (struct track* t = head; t; t = t->next) {
        if (t->has_meta_data) {
            printf("%3d: %s - ", count++, t->title);
            for (struct artist* a = t->artist; a; a = a->next) {
                printf("%s%s", a->name, a->next ? ", " : "");
	    }
            printf(" %s\n", t->playable ? "" : "(Unplayable)");
        }
        else {
            printf("%3d: N/A\n", count++);
	}
    }
}

void command_loop(struct despotify_session* ds) {
    bool loop = true;
    char *buf;
    struct playlist* rootlist = NULL;
    struct playlist* lastlist = NULL;

    

if (!rootlist) {
                    rootlist = despotify_get_stored_playlists(ds);
		}
                print_list_of_lists(rootlist);
		
    do {
	printf("\n> ");
        fflush(stdout);
	if((buf = wrapper_read_command()) == NULL) {
	    break;
	}

        /* list */
        if (!strncmp(buf, "list", 4)) {
            int num = 0;
            if(strlen(buf) > 5) {
		num = atoi(buf + 5);
	    }
			
            if (num) {
                struct playlist* p = get_playlist(rootlist, num);

                if (p) {
                    print_tracks(p->tracks);
                    lastlist = p;
                }
            }
           
        }

        /* play */
        else if (!strncmp(buf, "play", 4) || !strncmp(buf, "next", 4)) {
            if (!lastlist) {
		printf("No list to play from. Use 'list' or 'search' to select a list.\n");
                continue;
            }

            /* skip to track <num>, else play next */
            struct track* t;
            if (buf[4]) {
                int listoffset = atoi(buf + 5);
                t = lastlist->tracks;
                for (int i=1; i<listoffset && t; i++) {
                    t = t->next;
		}

                if (t) {
                    despotify_play(ds, t, true);
                    thread_play();
                }
                else {
		    printf("Invalid track number %d\n", listoffset);
		}
            }
            else {
                despotify_next(ds);
            }
        }

        /* stop */
        else if (!strncmp(buf, "stop", 4)) {
            thread_pause();
            despotify_stop(ds);
        }

        /* pause */
        else if (!strncmp(buf, "pause", 5)) {
            thread_pause();
        }

        /* resume */
        else if (!strncmp(buf, "resume", 5)) {
            thread_play();
        }

        /* quit (REMOVE)*/
        else if (!strncmp(buf, "quit", 4)) {
            loop = false;
        }
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
	    printf("%s - %s\n", t->artist->name,  t->title);
            break;
        }

        case DESPOTIFY_TIME_TELL:
            if ((int)(*((double*)data)) != seconds) {
                seconds = *((double*)data);
		printf("%d:%02d\n", seconds / 60, seconds % 60);
            }
            break;

        case DESPOTIFY_END_OF_PLAYLIST:
	    printf("End of playlist\n");
            thread_pause();
            break;
    }
}

int main(int argc, char** argv) {
    printf("Starting Raspify\n");
    setlocale(LC_ALL, "");

    if (argc < 3) {
	printf("Usage: %s <username> <password>\n", argv[0]);
        return 1;
    }
    
    if (!despotify_init()) {
	printf("despotify_init() failed\n");
        return 1;
    }

    struct despotify_session* ds = despotify_init_client(callback, NULL, true, true);
    if (!ds) {
	printf("despotify_init_client() failed\n");
        return 1;
    }

    pthread_create(&thread, NULL, &thread_loop, ds);

    if (!despotify_authenticate(ds, argv[1], argv[2])) {
        printf("Authentication failed: %s\n", despotify_get_error(ds));
        despotify_exit(ds);
        return 1;
    }
    printf("Logged in to Spotify\n");

    audio_device = audio_init();

    command_loop(ds);
    thread_exit();
    audio_exit(audio_device);
    despotify_exit(ds);
    
    if (!despotify_cleanup()) {
        printf("despotify_cleanup() failed\n");
        return 1;
    }

    return 0;
}

//(REMOVE/MODIFY)
char *wrapper_read_command(void) {
    static char stdin_buf[256] = { 0 };
    static int stdin_buf_len;
    static char *command = NULL;
	


    fd_set rfds;
    int max_fd = 0;
    char *ptr;
    int ret;

    if(command) {
	free(command);
	command = NULL;
    }

    for(;;) {
	FD_ZERO(&rfds);

		
	if(isatty(0)) {
	    FD_SET(0, &rfds);
	}
	
	if(select(max_fd + 1, &rfds, NULL, NULL, NULL) < 0) {
	    break;
	}
	
	if(FD_ISSET(0, &rfds)) {
	    ret = read(0, stdin_buf + stdin_buf_len, sizeof(stdin_buf) - stdin_buf_len - 1);
	    if(ret > 0) {
		stdin_buf_len += ret;
		stdin_buf[stdin_buf_len] = 0;
	    }
	}

	if((ptr = strchr(stdin_buf, '\n')) != NULL) {
	    *ptr++ = 0;
	    if(strlen(stdin_buf)) {
		command = strdup(stdin_buf);
            }		
	    stdin_buf_len -= ptr - stdin_buf;
	    memmove(stdin_buf, ptr, stdin_buf_len + 1);
	    if(command) {
		break;
	    }
	}
    }
    return command;
}

