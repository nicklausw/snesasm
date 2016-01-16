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
#include <cctype> // char checks
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
int append_token(unsigned int counter, int current_token); // add token to string


// token struct
typedef struct {
    int token_type; // type
    string token_i; // internals
} token;

// tokens vector
vector<token> tokens;

// token types
int tkUNDEF = 0; // fail
int tkNUM = 1; // number
int tkDIR = 2; // directive
int tkLB = 3; // label
int tkOP = 4; // opcode

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
    // the crazy part, tokens
    unsigned int counter = 0; // counter again
    int current_token = 0; // vector location value
    bool ct_used = false; // current token usage
    
    // we need the token vector big enough
    // for the first token, obviously
    tokens.resize(1);
    
    
    while (counter <= ins.length()) {
        // handle newlines
        if (ins[counter] == '\n') {
            if (ct_used == false) {
                counter++;
                continue;
            } else {
                current_token++;
                // handle vector size
                tokens.resize((current_token+1)*sizeof(token));
                
                // get ready for another loop
                ct_used = false;
                counter++;
                current_token++;
                continue;
            }
        }
        
        
        // no newline means do a token
        
        // skip space
        if (ins[counter] == ' ') {
            counter++;
            continue;
        }
        
        if (ins[counter] == '.') {
            // directive
            ct_used = true;
            tokens[current_token].token_type = tkDIR;
            counter++; // skip period
            
            counter = append_token(counter, current_token);
            
            // no need for a counter++ here, it's handled above.
            continue;
        } else if (isalpha(ins[counter])) {
            // opcode
            ct_used = true;
            cout << "opcode: " << ins[counter] << "\n";
            tokens[current_token].token_type = tkOP;
            
            counter = append_token(counter, current_token);
            
            // no need for a counter++ here, it's handled above.
            continue;
        }
        
        // be sure to move on to next symbol.
        counter++;
    }
    
    return success;
}


int append_token(unsigned int counter, int current_token)
{
    while (ins[counter] != ' ' && ins[counter] != '\n') {
                if (counter == ins.length())
                    break; // no overflows please!
                tokens[current_token].token_i.append(string(1, ins[counter]));
                counter++;
    }
    
    return counter;
}
