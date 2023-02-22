CC=clang-11
CFLAGS=-c -g
WALL=-Weverything -Wno-reserved-id-macro -Wno-disabled-macro-expansion
OBJECTS=obj/analyzer.o obj/buffers.o obj/global.o obj/printer.o obj/reader.o obj/threads.o obj/watchdog.o obj/destroyer.o obj/logger.o
TESTSOBJECTS=obj/tests_analyzer.o obj/tests_basic.o obj/tests_logger.o obj/tests_printer.o obj/tests_reader.o obj/tests_watchdog.o

output: $(OBJECTS) obj/main.o
	$(CC) $(OBJECTS) $(WALL) obj/main.o -o output

test: $(OBJECTS) $(TESTSOBJECTS)  obj/tests_main.o
	$(CC) $(OBJECTS) $(TESTSOBJECTS) $(WALL) obj/tests_main.o -o test

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) $(WALL) -o $@ $<

obj:
	mkdir -p $@

clean:
	clear
	rm -rf logs testLogs obj
	rm output test