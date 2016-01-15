/* snesasm

nicklausw's attempt at a portable
snes assembler */

#include <iostream> // basics
#include <fstream> // io
#include <string> // string
using namespace std; // print

// definitions
#define success 0
#define fail 1

// function declarations
void help(string prog_name); // help message
int snesasm(string in, string out); // the true main function
bool file_existent(string name); // file validity check

int main(int argc, char **argv)
{
    // args!
    if (argc != 3) {
        help(argv[0]);
        return fail;
    }
    
    // hand it all off to snesasm()
    if (snesasm(string(argv[1]), string(argv[2])) == fail) return fail;
    return success;
}

void help(string prog_name)
{
    cerr << "snesasm by nicklausw\n";
    cerr << "args: " << prog_name << " [in file] [out file]\n";
}

int snesasm(string in, string out)
{
    if (file_existent(in) == false) {
        cerr << "error: file " << in << " doesn't exist\n";
        return fail;
    }
    
    
    return success;
}

bool file_existent(string name)
{
    ifstream file(name.c_str());
    return file.good();
}
