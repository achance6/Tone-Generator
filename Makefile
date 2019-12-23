CC = gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra

all: render_tone render_song render_echo

render_tone: render_tone.o wave.o io.o
	$(CC) -o render_tone render_tone.o wave.o io.o -lm

render_song: render_song.o wave.o io.o
	$(CC) -o render_song render_song.o wave.o io.o -lm

render_echo: render_echo.o wave.o io.o
	$(CC) -o render_echo render_echo.o wave.o io.o -lm

wave.o: wave.h io.h
	$(CC) $(CFLAGS) -c wave.c 
io.o:
	$(CC) $(CFLAGS) -c io.c
render_tone.o:
	$(CC) $(CFLAGS) -c render_tone.c
render_song.o:
	$(CC) $(CFLAGS) -c render_song.c
render_echo.o:
	$(CC) $(CFLAGS) -c render_echo.c

clean:
	rm -f *.o render_tone render_song render_echo
