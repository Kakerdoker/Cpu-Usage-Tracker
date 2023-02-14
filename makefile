CC?=gcc
CFLAGS=-c
WALL=
OBJECTS=src/main.o

output: $(OBJECTS)
	$(CC) $(OBJECTS) -o output

main.o: src/main.c
	$(CC) $(CFLAGS) $(WALL) src/main.c

clean:
	rm $(OBJECTS) output