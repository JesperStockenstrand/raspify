#########################
# customise these
CFILES := mini.c libao.c audio.h
PROG := mini
CFLAGS := -Wall -Wextra -g -std=gnu99
LDFLAGS := -ldespotify -lao
########################

# -MMD generates dependencies while compiling
CFLAGS += -MMD
CC := gcc

OBJFILES := $(CFILES:.c=.o)
DEPFILES := $(CFILES:.c=.d)

$(PROG) : $(OBJFILES)
	$(LINK.o) $(LDFLAGS) -o $@ $^

clean :
	rm -f $(PROG) $(OBJFILES) $(DEPFILES)

-include $(DEPFILES)
