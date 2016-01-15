/* snesasm

nicklausw's attempt at a portable
snes assembler */

#include <iostream> // basics
using namespace std; // print

// function declarations
void help(string prog_name);

int main(int argc, char **argv)
{
    // args!
    if (argc != 4) {
        help(argv[0]);
        return 1;
    }
}

void help(string prog_name)
{
    cout << "snesasm by nicklausw\n";
    cout << "args: " << prog_name << " [in file] [out file]\n";
}
