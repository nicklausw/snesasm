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
#include <vector> // vectors
#include <cctype> // char checks
#include <cstdlib> // exit
#include <iterator> // vector to file
using namespace std; // print


// definitions
#define success 0
#define fail 1

// default to lorom
#define lorom 0
#define hirom 1


// function declarations
void help(string prog_name); // help message
int snesasm(string in, string out); // the true main function
bool file_existent(string name); // file existence check
void file_to_string(string file); // speaks for itself
int lexer(); // the wonderful lexer magic
int append_token(unsigned int counter, int current_token); // add token to string
int pass(); // pass function
int hint_next_token_type(unsigned int counter, string cur); // speaks for itself
string hint_next_token_dat(unsigned int counter, string cur); // same
int one_numeric_arg(string str, unsigned int counter, int range); // directives with one number arg
long parse_num(string num); // number parse
string str_tolower(string str); // lower all caps in string


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

// ranges
int rNONE = 0;
int r8 = 1;
int r16 = 2;
int r24 = 3;

string ins; // universal file string

// snes variables
bool compcheck_flag = false;
bool autoromsize_flag = false;
int romsize = 0;
int carttype = 0;
int licenseecode = 0;
int version = 0;
int lohirom = lorom;
int rombanks = 0;
long banksize = 0;

// temporary transfer variable
long tr = 0;


// the rom vector
vector<char> rom;


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
    if (pass() == fail) return fail; // pass
    
    ofstream outs(out);
    copy(rom.begin(), rom.end(), ostreambuf_iterator<char>(outs));
    
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
    
    
    while (counter < ins.length()) {
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
            
            // no case sensitivity
            tokens[current_token].token_i = str_tolower(tokens[current_token].token_i);
            
            // no need for a counter++ here, it's handled above.
            continue;
        } else if (isalpha(ins[counter])) {
            // opcode
            ct_used = true;
            tokens[current_token].token_type = tkOP;
            
            counter = append_token(counter, current_token);
            
            // no need for a counter++ here, it's handled above.
            continue;
        } else if (isdigit(ins[counter]) || ins[counter] == '$' || ins[counter] == '%') {
            // number
            ct_used = true;
            tokens[current_token].token_type = tkNUM;
            
            counter = append_token(counter, current_token);
            
            // no case sensitivity
            tokens[current_token].token_i = str_tolower(tokens[current_token].token_i);
            
            // no need for a counter++ here, it's handled above.
            continue;
        } else {
            // unknown
            ct_used = true;
            tokens[current_token].token_type = tkUNDEF;
            
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

int pass()
{    
    for (unsigned int counter = 0; counter < (tokens.size())/sizeof(token); counter++) {
        if (tokens[counter].token_type == tkDIR) {
            if (tokens[counter].token_i == "compcheck") {
                compcheck_flag = true;
            } else if (tokens[counter].token_i == "autoromsize") {
                autoromsize_flag = true;
            } else if (tokens[counter].token_i == "romsize") {
                counter = one_numeric_arg("romsize", counter, r8); romsize = tr;
            } else if (tokens[counter].token_i == "carttype") {
                counter = one_numeric_arg("carttype", counter, r8); carttype = tr;
            } else if (tokens[counter].token_i == "licenseecode") {
                counter = one_numeric_arg("licenseecode", counter, r8); licenseecode = tr;
            } else if (tokens[counter].token_i == "version") {
                counter = one_numeric_arg("version", counter, r8); version = tr;
            } else if (tokens[counter].token_i == "banksize") {
                counter = one_numeric_arg("banksize", counter, r16); banksize = tr;
                
                if (rombanks != 0) {
                    // go ahead and set size of output
                    rom.resize(rombanks*banksize);
                }
            } else if (tokens[counter].token_i == "rombanks") {
                // not sure about rombanks limit
                counter = one_numeric_arg("rombanks", counter, rNONE); rombanks = tr;
                
                if (banksize != 0) {
                    // go ahead and set size of output
                    rom.resize(rombanks*banksize);
                }
            } else if (tokens[counter].token_i == "lorom") {
                lohirom = lorom;
            } else if (tokens[counter].token_i == "hirom") {
                lohirom = hirom;
            } else {
                cout << "error: unknown directive \"" << tokens[counter].token_i << "\"\n";
                return fail;
            }
        } else if (tokens[counter].token_type == tkNUM) {
            cerr << "error: loose num " << tokens[counter].token_i << '\n';
            return fail;
        } else if (tokens[counter].token_type == tkOP) {
            cerr << "error: opcodes aren't implemented yet, sorry.\n";
            return fail;
        } else {
            cerr << "error: unknown symbol \"" << tokens[counter].token_i << "\"\n";
            return fail;
        }
    }
    
    return success;
}


int hint_next_token_type(unsigned int counter, string cur)
{
    if (counter >= tokens.size()) {
        cerr << "error: " << cur << " requires args\n";
        exit(fail);
    }
    
    return tokens[counter+1].token_type;
}


string hint_next_token_dat(unsigned int counter, string cur)
{
    if (counter >= tokens.size()) {
        cerr << "error: " << cur << " requires args\n";
        exit(fail);
    }
    
    return tokens[counter+1].token_i;
}


int one_numeric_arg(string str, unsigned int counter, int range)
{
    if (hint_next_token_type(counter, tokens[counter].token_i) != tkNUM) {
        cerr << "error: " << str << " expects numeric args\n";
        exit(fail);
    } else {
        tr = parse_num(hint_next_token_dat(counter, tokens[counter].token_i));
        
        if (range == r8) {
            if (tr > 0xFF) {
                cerr << "error: " << str << " requires 8-bit args\n";
                exit(fail);
            }
        } else if (range == r16) {
            if (tr > 0xFFFF) {
                cerr << "error: " << str << " requires 16-bit args\n";
                exit(fail);
            }
        }
        
        counter++;
    }
    
    return counter;
}


long parse_num(string num)
{
    string without_sym = num; // declare without symbol
    
    // make 'without symbol' true
    if (!isdigit(without_sym[0])) {
        without_sym.erase(without_sym.begin());
    }
    
    // check for invalid symbols
    for (unsigned int checkn = 0; checkn < without_sym.length(); checkn++) {
        if (isdigit(without_sym[checkn])) continue;
        
        // not a digit...
        if (num[0] != '$') {
            cerr << "error: invalid symbol in number\n";
            exit(fail);
        } else {
            switch (without_sym[checkn]) {
                case 'a': continue;
                case 'b': continue;
                case 'c': continue;
                case 'd': continue;
                case 'e': continue;
                case 'f': continue;
                default:
                    // invalid symbol
                    cerr << "error: invalid symbol in number\n";
                    exit(fail);
            }
        }
    }
    
    
    char *chararray = const_cast<char*>(without_sym.c_str()); // turn without_sym into char array
    
    switch (num[0]) {
        case '%':
            // binary
            return strtol(chararray, &chararray, 2);
        case '$':
            // hex
            return strtol(chararray, &chararray, 16);
    }
    
    return strtol(chararray, &chararray, 10);
}


string str_tolower(string str)
{ 
    for (unsigned int counter = 0; counter < str.length(); counter++) {
        if (isalpha(str[counter])) {
            str[counter] = tolower(str[counter]);
        }
    }
    
    return str;
}
