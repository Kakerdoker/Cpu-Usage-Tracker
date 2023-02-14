CC?=gcc
CFLAGS=-c
WALL=
OBJECTS=main.o analyzer.o buffers.o global.o printer.o reader.o threads.o

output: $(OBJECTS)
	$(CC) $(OBJECTS) -o output

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

clean:
	rm $(OBJECTS) output