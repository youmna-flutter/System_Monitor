CC = gcc
CFLAGS = -Wall -Wextra -I/usr/include/ncurses
LIBS = -lm -lncurses

all: sysmonitor

sysmonitor: src/main.c src/process.c src/stats.c src/ui.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f sysmonitor