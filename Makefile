CC ?= cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700
OBJS = server.o http.o files.o cgi.o
all: sws
sws: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
%.o: %.c sws.h
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f sws $(OBJS)
.PHONY: all clean
