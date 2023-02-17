CC?=gcc
CFLAGS=-c
WALL=-Wall
OBJECTS=obj/analyzer.o obj/buffers.o obj/global.o obj/printer.o obj/reader.o obj/threads.o obj/watchdog.o obj/destroyer.o

output: $(OBJECTS) obj/main.o
	$(CC) $(OBJECTS) obj/main.o -o output

test: $(OBJECTS) obj/tests.o
	$(CC) $(OBJECTS) obj/tests.o -o test

obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(WALL) -o $@ $<

clean:
	rm $(OBJECTS) obj/tests.o obj/main.o output test