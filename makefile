CC?=gcc
CFLAGS=-c
WALL=-Wall
OBJECTS=analyzer.o buffers.o global.o printer.o reader.o threads.o

    #######################

output: $(OBJECTS) main.o
	$(CC) $(OBJECTS) main.o -o output

test: $(OBJECTS) tests.o
	$(CC) $(OBJECTS) tests.o -o test

    #######################

tests.o: src/main.c
	$(CC) $(CFLAGS) $(WALL) tests/tests.c

main.o: src/main.c
	$(CC) $(CFLAGS) $(WALL) src/main.c

analyzer.o: src/analyzer.c
	$(CC) $(CFLAGS) $(WALL) src/analyzer.c

buffers.o: src/buffers.c
	$(CC) $(CFLAGS) $(WALL) src/buffers.c

global.o: src/global.c
	$(CC) $(CFLAGS) $(WALL) src/global.c

printer.o: src/printer.c
	$(CC) $(CFLAGS) $(WALL) src/printer.c

reader.o: src/reader.c
	$(CC) $(CFLAGS) $(WALL) src/reader.c

threads.o: src/threads.c
	$(CC) $(CFLAGS) $(WALL) src/threads.c

    #######################

clean:
	rm $(OBJECTS) tests.o main.o output test