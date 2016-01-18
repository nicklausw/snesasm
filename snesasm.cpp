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

// default to slowrom
#define slowrom 0
#define fastrom 1


// token struct
typedef struct {
    int token_type; // type
    string token_i; // internals
} token;


// function declarations
void help(string prog_name); // help message
int snesasm(string in, string out); // the true main function
bool file_existent(string name); // file existence check
void file_to_string(string file); // speaks for itself
int lexer(); // the wonderful lexer magic
int append_token(unsigned int counter, int current_token); // add token to string
int pass(); // pass function
token hint_next_token(unsigned int counter, string cur); // speaks for itself
int numeric_arg(string str, unsigned int counter, int range); // directives with one number arg
long parse_num(string num); // number parse
string str_tolower(string str); // lower all caps in string
void write_byte(unsigned char byte); // write byte to rom
void new_label(string name, long val); // add a new label
void snes(); // do snes opcodes!


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
unsigned char romsize = 0;
unsigned char carttype = 0;
unsigned char licenseecode = 0;
unsigned char version = 0;
unsigned char lohirom = lorom;
unsigned char sfrom = slowrom;
unsigned char rombanks = 0;
long banksize = 0;
int cur_bank = 0;
int base = 0x8000;
long org = 0x8000;

// define switches
bool rombanks_defined = false;
bool banksize_defined = false; 

// temporary transfer variable
long tr = 0;


// the rom vector
vector<unsigned char> rom;

// the labels struct
typedef struct {
    string name;
    long val;
} label;

vector<label> labels;
int label_count = 0;


// opcode struct
// only works with no-args
typedef struct {
    string name;
    unsigned char byte;
    bool no_arg;
} opcode;


// opcode list
opcode opcodes[] = {
    {"xce", 0xFB, true},
    {"clc", 0x18, true}
};


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
    
    snes();
    
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
        
        if (ins[counter] == 0x0D) {
            // CR LF
            counter++;
            continue;
        }
        
        
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
    unsigned int str_length = ins.length() - 1;
    
    while (ins[counter] != ' ' && ins[counter] != '\n' && ins[counter] != 0x0D) {
        if (counter == str_length) {
            counter++;
            break; // no overflows please!
        }
        
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
                counter = numeric_arg("romsize", counter, r8); romsize = tr;
            } else if (tokens[counter].token_i == "carttype") {
                counter = numeric_arg("carttype", counter, r8); carttype = tr;
            } else if (tokens[counter].token_i == "licenseecode") {
                counter = numeric_arg("licenseecode", counter, r8); licenseecode = tr;
            } else if (tokens[counter].token_i == "version") {
                counter = numeric_arg("version", counter, r8); version = tr;
            } else if (tokens[counter].token_i == "org") {
                counter = numeric_arg("org", counter, r16); org = tr;
            } else if (tokens[counter].token_i == "banksize") {
                counter = numeric_arg("banksize", counter, r16); banksize = tr;
                banksize_defined = true;
                
                if (rombanks != 0) {
                    // go ahead and set size of output
                    rom.resize(rombanks*banksize);
                }
            } else if (tokens[counter].token_i == "rombanks") {
                // not sure about rombanks limit
                counter = numeric_arg("rombanks", counter, rNONE); rombanks = tr;
                rombanks_defined = true;
                
                if (banksize != 0) {
                    // go ahead and set size of output
                    rom.resize(rombanks*banksize);
                }
            } else if (tokens[counter].token_i == "bank") {
                counter = numeric_arg("bank", counter, rNONE); cur_bank = tr;
                
                if (rombanks_defined == false) {
                    cerr << "error: rombanks needs to be defined before .bank\n";
                    return fail;
                } else if (banksize_defined == false) {
                    cerr << "error: banksize needs to be defined before .bank\n";
                    return fail;
                }
                
                if (cur_bank >= rombanks) {
                    cerr << "error: can't go to bank higher than maximum banks\n";
                    return fail;
                } else {
                    org = (banksize*cur_bank)+base;
                }
            } else if (tokens[counter].token_i == "db") {
                while (hint_next_token(counter, "db").token_type == tkNUM) {
                    counter = numeric_arg("db", counter, r8); write_byte(tr & 0xFF);
                }
            } else if (tokens[counter].token_i == "dw") {
                while (hint_next_token(counter, "dw").token_type != tkUNDEF) {
                    counter = numeric_arg("dw", counter, r16);
                    write_byte(tr >> 8);
                    write_byte(tr & 0xFF);
                }
            } else if (tokens[counter].token_i == "lorom") {
                lohirom = lorom;
            } else if (tokens[counter].token_i == "hirom") {
                lohirom = hirom;
            } else if (tokens[counter].token_i == "slowrom") {
                sfrom = slowrom;
            } else if (tokens[counter].token_i == "fastrom") {
                sfrom = fastrom;
            } else {
                cout << "error: unknown directive " << tokens[counter].token_i << "\n";
                return fail;
            }
        } else if (tokens[counter].token_type == tkNUM) {
            cerr << "error: loose num " << tokens[counter].token_i << "\n";
            return fail;
        } else if (tokens[counter].token_type == tkOP) {
            // opcode
            int match_count = 0;
            for (unsigned int opcounter = 0; opcounter < sizeof(opcodes)/sizeof(opcode); opcounter++) {
                if (tokens[counter].token_i == opcodes[opcounter].name) {
                    match_count++;
                } else continue;
                
                // it's a match
                // ...but it has to have no args for now.
                if (opcodes[opcounter].no_arg == false) {
                    cerr << "error: no-arg opcodes only for now.\n";
                }
                write_byte(opcodes[opcounter].byte);
            }
            
            if (!match_count) {
                // no matches
                // is it a label?
                if (tokens[counter].token_i[tokens[counter].token_i.size() - 1] == ':') {
                    tokens[counter].token_i.pop_back();
                    new_label(tokens[counter].token_i, org);
                } else {
                    cerr << "error: unknown opcode " << tokens[counter].token_i << "\n";
                    return fail;
                }
            }
        } else {
            cerr << "error: unknown symbol " << tokens[counter].token_i << "\n";
            return fail;
        }
    }
    
    return success;
}


token hint_next_token(unsigned int counter, string cur)
{
    if (counter >= tokens.size()) {
        cerr << "error: " << cur << " requires args\n";
        exit(fail);
    }
    
    return tokens[counter+1];
}


int numeric_arg(string str, unsigned int counter, int range)
{
    unsigned int label_search;
    int label_count = 0;
    
    if (hint_next_token(counter, tokens[counter].token_i).token_type == tkOP) { // if it's not number
        for (label_search = 0; label_search < labels.size(); label_search++) { // look for a label!
            if (labels[label_search].name == hint_next_token(counter, tokens[counter].token_i).token_i) {
                tr = labels[label_search].val; // it's a match!
                label_count++; // no need for error.
                break;
            }
        }
        
        if (!label_count) { // if no match was found
            cerr << "error: no such label " << hint_next_token(counter, tokens[counter].token_i).token_i << "\n";
            exit(fail);
        }
    } else if (hint_next_token(counter, tokens[counter].token_i).token_type == tkNUM) {
        tr = parse_num(hint_next_token(counter, tokens[counter].token_i).token_i);
    } else {
        cerr << "error: " << str << " expects numeric args\n";
        exit(fail);
    }
     
 
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


void write_byte(unsigned char byte)
{
    // the surprisingly simplest function of all!
    rom[org-base] = byte; org++;
}


void new_label(string name, long val)
{
    labels.push_back({name, val});
}


void snes()
{
    unsigned char build_byte = 0x00;
    
    if (lohirom == hirom) {
        build_byte += 0x1;
    }
    
    if (sfrom == slowrom) {
        build_byte += 0x20;
    }
    
    base = 0x8000;
    
    org = 0xFFD5;
    write_byte(build_byte);
    write_byte(carttype);
    write_byte(romsize);
    write_byte(0);
    write_byte(0);
    write_byte(licenseecode);
    write_byte(version);
    
    // clear checksum bytes
    write_byte(0);
    write_byte(0);
    write_byte(0);
    write_byte(0);
    
    
    // the weird part, checksum calculation.
    auto checksum = 0ULL;
    auto inverse = 0ULL;
    for (auto counter = 0ULL; counter < rom.size(); counter++) {
        checksum += rom[counter];
    }
    
    
    checksum += 0x1FE; // add inverse to checksum
    checksum &= 0xFFFF; // lower 16 bytes
    inverse = checksum ^ 0xFFFF; // 0xFFFF - checksum
    
    
    // write to rom
    org = 0xFFDC;
    write_byte(inverse & 0xFF);
    write_byte(inverse >> 8);
    write_byte(checksum & 0xFF);
    write_byte(checksum >> 8);
}