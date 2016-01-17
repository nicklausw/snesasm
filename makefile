all:
	g++ -std=c++0x -pedantic-errors -Wall -Wextra -Werror -o snesasm snesasm.cpp
	./snesasm snesasm.s snesasm.sfc
