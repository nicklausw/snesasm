all:
	g++ -std=c++0x -pedantic-errors -Wall -o snesasm snesasm.cpp
	./snesasm snesasm.s snesasm.sfc
