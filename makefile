all:
	g++ -std=c++0x -Os -pedantic-errors -Wall -Wextra -Werror -o snesasm snesasm.cpp
	./snesasm legit_test.s legit_test.sfc
