/* snesasm

nicklausw's attempt at a portable
snes assembler */

#include <iostream> // basics
using namespace std; // print

// definitions
#define success 0
#define fail 1

// function declarations
void help(string prog_name);
int snesasm(string in, string out);

int main(int argc, char **argv)
{
    // args!
    if (argc != 3) {
        help(argv[0]);
        return fail;
    }
    
    // hand it all off to snesasm()
    if (snesasm(argv[1], argv[2]) == fail) return fail;
    return success;
}

void help(string prog_name)
{
    cout << "snesasm by nicklausw\n";
    cout << "args: " << prog_name << " [in file] [out file]\n";
}

int snesasm(string in, string out)
{
    // do it all!
    return success;
}
