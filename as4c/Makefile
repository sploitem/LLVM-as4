all: parser

scanner.cpp: scanner.l
	flex -o scanner.cpp scanner.l

parser.cpp parser.hpp: parser.y
	bison -d -o parser.cpp parser.y

parser: main.cpp scanner.cpp parser.cpp parser.hpp
	g++ -std=c++11 main.cpp scanner.cpp parser.cpp `llvm-config-7 --cppflags --ldflags --libs --system-libs all` -o parser

test:
	gcc test.c outputs.o -o test

clean:
	rm -f test parser scanner.cpp parser.cpp parser.hpp outputs.o
