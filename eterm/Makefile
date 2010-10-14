CFLAGS += -std=gnu99 -DPC_TESTS

test: eterm.a(eterm.o) sgparsers.o orc32parsers.o tests.o
	gcc -o $@ tests.o sgparsers.o orc32parsers.o eterm.a

clean:
	rm *.a *.o test
