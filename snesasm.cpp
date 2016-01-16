/* snesasm

nicklausw's attempt at a portable
snes assembler 

licensed under the isc license. you
should have gotten a copy with snesasm,
if not then visit http://nicklausw.github.io/isc.txt 

enjoy. */

#include <iostream> // basics
#include <fstream> // io
#include <string> // string
#include <sstream> // file streams
#include <vector> // vectors
using namespace std; // print

// definitions
#define success 0
#define fail 1

// function declarations
void help(string prog_name); // help message
int snesasm(string in, string out); // the true main function
bool file_existent(string name); // file existence check
void file_to_string(string file); // speaks for itself
int lexer(); // the wonderful lexer magic

// token struct
typedef struct {
    int token_type; // type
    string token_i; // internals
} token;

// tokens vector
vector<token> tokens;

string ins; // universal file string

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
    
    // read file into string
    file_to_string(in);
    
    // lexer magic
    if (lexer() == fail) return fail;
    
    return success;
}

bool file_existent(string name)
{
    ifstream file(name.c_str());
    return file.good();
}

void file_to_string(string file)
{
    ifstream in(file.c_str()); // file stream
    string str; // temporary build-up string
    char in_c; // char for comparison
    unsigned int counter = 0; // basic counter
    
    // clear ins
    ins.clear();
    
    // read into new_str converting tabs to spaces
    while (!in.eof()) {
        in.get(in_c);
        if (in_c == '\t') {
            ins.append(" ");
        } else {
            ins.append(string(1, in_c));
        }
    }
    
    str = ins;
    
    // simplify multiple spaces to one space
    ins.clear();
    
    while (counter <= str.length()) {
        if (str[counter] == ' ') {
            ins.append(" ");
            while (str[counter] == ' ') {
                counter++;
                if (counter == str.length())
                    break;
            }
        } else {
            ins.append(string(1, str[counter]));
            counter++;
        }
    }
    
    // do same to remove comments
    ins.clear();
    counter = 0;
    
    while (counter <= str.length()) {
        if (str[counter] == ';') {
            ins.append("\n");
            while (str[counter] != '\n') {
                counter++;
                if (counter == str.length())
                    break;
            }
        } else {
            ins.append(string(1, str[counter]));
            counter++;
        }
    }
}

int lexer()
{
    return success;
}
