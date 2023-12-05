PREFIX?=/usr/X11R6
CFLAGS?=-Os -pedantic -Wall

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include main.c -L$(PREFIX)/lib -lX11 -o build/astwm

test:
	Xephyr -ac -br -noreset -screen 800x600 :1 &
	sleep 1
	DISPLAY=:1 ./build/astwm &
	DISPLAY=:1 xclock &
	DISPLAY=:1 xterm

clean:
	rm -f ./build/*