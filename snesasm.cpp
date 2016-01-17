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
#include <cstdlib> // exit
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
int pass_1(); // first pass: get directives, labels, defines
int pass_2(); // second pass: calculate stuff, fix stuff, write to rom
int hint_next_token_type(unsigned int counter, string cur); // speaks for itself
string hint_next_token_dat(unsigned int counter, string cur); // same
int one_numeric_arg(string str, unsigned int counter); // directives with one number arg


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
int tkOP = 3; // opcode

string ins; // universal file string

// snes variables
bool compcheck_flag = false;
bool autoromsize_flag = false;
int romsize = 0;
int carttype = 0;
int licenseecode = 0;
int version = 0;

// temporary transfer variable
int tr = 0;


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
    
    
    file_to_string(in); // read file
    if (lexer() == fail) return fail; // lexer magic
    if (pass_1() == fail) return fail; // pass 1
    if (pass_2() == fail) return fail; // pass 2
    
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
        if (ins[counter] == '\n' || ins[counter] == ' ') {
            if (ct_used == false) {
                counter++;
                continue;
            } else {
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
            tokens[current_token].token_type = tkOP;
            
            counter = append_token(counter, current_token);
            
            // no need for a counter++ here, it's handled above.
            continue;
        } else if (isdigit(ins[counter])) {
            // number
            ct_used = true;
            tokens[current_token].token_type = tkNUM;
            
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

int pass_1()
{
    for (unsigned int counter = 0; counter < (tokens.size())/sizeof(token); counter++) {
        if (tokens[counter].token_type == tkDIR) {
            if (tokens[counter].token_i == "compcheck") {
                compcheck_flag = true;
            } else if (tokens[counter].token_i == "autoromsize") {
                autoromsize_flag = true;
            } else if (tokens[counter].token_i == "romsize") {
                counter = one_numeric_arg("romsize", counter); romsize = tr;
            } else if (tokens[counter].token_i == "carttype") {
                counter = one_numeric_arg("carttype", counter); carttype = tr;
            } else if (tokens[counter].token_i == "licenseecode") {
                counter = one_numeric_arg("licenseecode", counter); licenseecode = tr;
            } else if (tokens[counter].token_i == "version") {
                counter = one_numeric_arg("version", counter); version = tr;
            } else {
                cout << "error: unknown directive \"" << tokens[counter].token_i << "\"\n";
                return fail;
            }
        } else if (tokens[counter].token_type == tkNUM) {
            cout << "error: loose num " << tokens[counter].token_i << '\n';
            return fail;
        } else if (tokens[counter].token_type == tkOP) {
            cout << "error: opcodes aren't implemented yet, sorry.\n";
            return fail;
        } else {
            cout << "error: unknown symbol \"" << tokens[counter].token_i << "\"\n";
            return fail;
        }
    }
    
    return success;
}


int pass_2()
{
    return success;
}


int hint_next_token_type(unsigned int counter, string cur)
{
    if (counter >= tokens.size()) {
        cout << "error: " << cur << " requires args\n";
        exit(fail);
    }
    
    return tokens[counter+1].token_type;
}


string hint_next_token_dat(unsigned int counter, string cur)
{
    if (counter >= tokens.size()) {
        cout << "error: " << cur << " requires args\n";
        exit(fail);
    }
    
    return tokens[counter+1].token_i;
}


int one_numeric_arg(string str, unsigned int counter)
{
    if (hint_next_token_type(counter, tokens[counter].token_i) != tkNUM) {
        cout << "error: " << str << " expects numeric args\n";
        exit(fail);
    } else {
        string out_hint = hint_next_token_dat(counter, tokens[counter].token_i);
        tr = atoi(out_hint.c_str());
        counter++;
    }
    
    return counter;
}
