CC?=gcc
CFLAGS=-c
WALL=-Wall
OBJECTS=obj/analyzer.o obj/buffers.o obj/global.o obj/printer.o obj/reader.o obj/threads.o obj/watchdog.o obj/destroyer.o obj/logger.o

output: $(OBJECTS) obj/main.o
	$(CC) $(OBJECTS) obj/main.o -o output

test: $(OBJECTS) obj/tests.o
	$(CC) $(OBJECTS) obj/tests.o -o test

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) $(WALL) -o $@ $<

obj:
	mkdir -p $@

clean:
	rm -rf logs testLogs obj
	rm output test