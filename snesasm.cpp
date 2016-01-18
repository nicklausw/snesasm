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
void raw_data(int range, string name); // raw data function
token hint_next_token(); // speaks for itself
void directive_arg(string str, int range); // directives with one number arg
long parse_num(string num); // number parse
string str_tolower(string str); // lower all caps in string
void write_byte(unsigned char byte); // write byte to rom
void new_label(string name, long val); // add a new label
void new_unsolved(string name, long loc, int type); // add a new unsolved label
void new_unsolved_opcode(string name, long loc, int type, // add a new unsolved opcode?
             unsigned char byte, unsigned char b8, unsigned char b16, unsigned char b24);
int solve_unsolveds(); // solve previously unsolved stuff
void snes(); // do snes opcodes!


// tokens vector
vector<token> tokens;

// token types
int tkUNDEF = 0; // fail
int tkNUM = 1; // number
int tkDIR = 2; // directive
int tkOP = 3; // opcode
int tkLB = 4; // label
int tkARG = 5; // opcode arg
int tkNL = 6; // newline
int tkCOMMA = 7; // comma


// ranges
int rNONE = 0;
int r8 = 1;
int r16 = 2;
int r24 = 3;
int rOP = 6;

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
unsigned long banksize = 0;
unsigned long cur_bank = 0;
unsigned long base = 0x8000;
unsigned long org = 0x8000;

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


// the unsolved struct
typedef struct {
  string name;
  long loc;
  int type;
  bool opcode;
  unsigned char byte;
  unsigned char b8;
  unsigned char b16;
  unsigned char b24;
} unsolved;

vector<unsolved> unsolveds;

// unsolved types
int tDB = 0;
int tDW = 1;
int tOP = 5;


// counter for pass
unsigned long pass_counter = 0;


// opcode struct
// only works with no-args
typedef struct {
  string name;
  int no_arg;     // xxx
  int one8;       // xxx $00
  int one16;      // xxx $1010
  int one24;      // xxx $101010
  int ind;        // xxx ($10)
  int lit;        // xxx #$10
  int x8;         // xxx $10,x
  int x16;        // xxx $1010,x
  int x24;        // xxx $101010,x
  int relative8;  // xxx -3
  int relative16; // xxx -356
} opcode;


// we need to shorthand...a lot
#define t true
#define f false

// opcode list
opcode opcodes[] = {
  {"adc", -1},
  {"and", -1},
  {"asl", -1},
  {"bcc", -1},
  {"bcs", -1},
  {"beq", -1},
  {"bit", -1},
  {"bmi", -1},
  {"bne", -1},
  {"bpl", -1},
  {"bra", -1},
  {"brk", 0x00},
  {"brl", -1},
  {"bvc", -1},
  {"bvs", -1},
  {"clc", 0x18},
  {"cld", 0xD8},
  {"cli", 0x58},
  {"clv", 0xB8},
  {"cmp", -1},
  {"cop", -1},
  {"cpx", -1},
  {"cpy", -1},
  {"dec", -1},
  {"dex", 0xCA},
  {"dey", 0x88},
  {"eor", -1},
  {"inc", -1},
  {"inx", 0xE8},
  {"iny", 0xC8},
  {"jmp", -1},
  {"jsr", -1},
  {"lda", -1},
  {"ldx", -1},
  {"ldy", -1},
  {"lsr", -1},
  {"mvn", -1},
  {"mvp", -1},
  {"nop", 0xEA},
  {"ora", -1},
  {"pea", -1},
  {"pei", -1},
  {"per", -1},
  {"pha", 0x48},
  {"phb", 0x8B},
  {"phd", 0x0B},
  {"phk", 0x4B},
  {"php", 0x08},
  {"phx", 0xDA},
  {"phy", 0x5A},
  {"pla", 0x68},
  {"plb", 0xAB},
  {"pld", 0x2B},
  {"plp", 0x28},
  {"plx", 0xFA},
  {"ply", 0x7A},
  {"rep", -1},
  {"rol", -1},
  {"ror", -1},
  {"rti", 0x40},
  {"rtl", 0x6B},
  {"rts", 0x60},
  {"sbc", -1},
  {"sec", 0x38},
  {"sed", 0xF8},
  {"sei", 0x78},
  {"sep", -1},
  {"sta", -1},
  {"stp", 0xDB},
  {"stx", -1},
  {"sty", -1},
  {"stz", -1},
  {"tax", 0xAA},
  {"tay", 0xA8},
  {"tcd", 0x5B},
  {"tcs", 0x1B},
  {"tdc", 0x7B},
  {"trb", -1},
  {"tsc", 0x3B},
  {"tsx", 0xBA},
  {"txa", 0x8A},
  {"txs", 0x9A},
  {"txy", 0x9B},
  {"tya", 0x98},
  {"tyx", 0xBB},
  {"wai", 0xCB},
  {"wdm", 0x42},
  {"xba", 0xEB},
  {"xce", 0xFB}
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
  if (solve_unsolveds() == fail) return fail; // solve unknown labels
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
  
  // prevent bug with last line being screwed up
  // by erasing very last character (copied twice)
  str.pop_back();
  
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
    if (ins[counter] == '\n') {
      if (ct_used == false) {
        counter++;
        current_token++;
        tokens.resize((current_token+2)*sizeof(token));
        tokens[current_token].token_type = tkNL;
      } else {
        // handle vector size
        tokens.resize((current_token+1)*sizeof(token));
        
        // get ready for another loop
        ct_used = false;
        counter++;
        current_token++;
        tokens.resize((current_token+2)*sizeof(token));
        tokens[current_token].token_type = tkNL;
        current_token++;
      }
      continue;
    }
    
    if (ins[counter] == ',') {
      if (ct_used == false) {
        counter++;
        current_token++;
        tokens.resize((current_token+2)*sizeof(token));
        tokens[current_token].token_type = tkCOMMA;
      } else {
        // handle vector size
        tokens.resize((current_token+1)*sizeof(token));
        
        // get ready for another loop
        ct_used = false;
        counter++;
        current_token++;
        tokens.resize((current_token+2)*sizeof(token));
        tokens[current_token].token_type = tkCOMMA;
        current_token++;
      }
      continue;
    }
    
    if (ins[counter] == ' ') {
      if (ct_used == false) {
        counter++;
      } else {
        // handle vector size
        tokens.resize((current_token+1)*sizeof(token));
        
        // get ready for another loop
        ct_used = false;
        counter++;
        current_token++;
      }
      continue;
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
      
      if (tokens[current_token].token_i[tokens[current_token].token_i.length() - 1] == ':')
        tokens[current_token].token_type = tkLB; // it's a label
      
      // no need for a counter++ here, it's handled above.
      continue;
    } else if (isdigit(ins[counter]) || ins[counter] == '$' || ins[counter] == '%' || ins[counter] == '-') {
      // number
      ct_used = true;
      tokens[current_token].token_type = tkNUM;
      
      counter = append_token(counter, current_token);
      
      // no case sensitivity
      tokens[current_token].token_i = str_tolower(tokens[current_token].token_i);
      
      // no need for a counter++ here, it's handled above.
      continue;
    } else if (ins[counter] == '#' || ins[counter] == '(') {
      // opcode arg
      ct_used = true;
      tokens[current_token].token_type = tkARG;
      
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
  
  while (ins[counter] != ' ' && ins[counter] != '\n' && ins[counter] != 0x0D && ins[counter] != ',') {
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
  for (pass_counter = 0; pass_counter < (tokens.size())/sizeof(token); pass_counter++) {
    if (tokens[pass_counter].token_type == tkDIR) {
      // directive
      if (tokens[pass_counter].token_i == "compcheck") {
        compcheck_flag = true;
      } else if (tokens[pass_counter].token_i == "autoromsize") {
        autoromsize_flag = true;
      } else if (tokens[pass_counter].token_i == "romsize") {
        directive_arg("romsize", r8); romsize = tr;
      } else if (tokens[pass_counter].token_i == "carttype") {
        directive_arg("carttype", r8); carttype = tr;
      } else if (tokens[pass_counter].token_i == "licenseecode") {
        directive_arg("licenseecode", r8); licenseecode = tr;
      } else if (tokens[pass_counter].token_i == "version") {
        directive_arg("version", r8); version = tr;
      } else if (tokens[pass_counter].token_i == "org") {
        directive_arg("org", r16); org = tr;
        
        if (banksize_defined == false || rombanks_defined == false) {
          cerr << "error: rombanks and banksize need to be defined before org\n";
          return fail;
        }
      } else if (tokens[pass_counter].token_i == "banksize") {
        directive_arg("banksize", r16); banksize = tr;
        banksize_defined = true;
        
        if (rombanks != 0) {
          // go ahead and set size of output
          rom.resize(rombanks*banksize);
        }
      } else if (tokens[pass_counter].token_i == "rombanks") {
        // not sure about rombanks limit
        directive_arg("rombanks", rNONE); rombanks = tr;
        rombanks_defined = true;
        
        if (banksize != 0) {
          // go ahead and set size of output
          rom.resize(rombanks*banksize);
        }
      } else if (tokens[pass_counter].token_i == "bank") {
       directive_arg("bank", rNONE); cur_bank = tr;
        
        // reset org
        org = base*(cur_bank+1);
        
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
      } else if (tokens[pass_counter].token_i == "db") {
          raw_data(r8, "db");
      } else if (tokens[pass_counter].token_i == "dw") {
          raw_data(r16, "dw");
      } else if (tokens[pass_counter].token_i == "lorom") {
        lohirom = lorom;
      } else if (tokens[pass_counter].token_i == "hirom") {
        lohirom = hirom;
      } else if (tokens[pass_counter].token_i == "slowrom") {
        sfrom = slowrom;
      } else if (tokens[pass_counter].token_i == "fastrom") {
        sfrom = fastrom;
      } else {
        cout << "error: unknown directive " << tokens[pass_counter].token_i << "\n";
        return fail;
      }
    } else if (tokens[pass_counter].token_type == tkNUM || tokens[pass_counter].token_type == tkARG) {
      cerr << "error: loose phrase " << tokens[pass_counter].token_i << "\n";
      return fail;
    } else if (tokens[pass_counter].token_type == tkOP) {
      // opcode
      int match_count = 0;
      token next_tok = hint_next_token();
      
      if (banksize_defined == false || rombanks_defined == false) {
        cerr << "error: rombanks and banksize need to be defined before opcodes\n";
        return fail;
      }
      
      for (unsigned int opcounter = 0; opcounter < sizeof(opcodes)/sizeof(opcode); opcounter++) {
        if (tokens[pass_counter].token_i == opcodes[opcounter].name) {
          match_count++;
        } else continue;
        
        // it's a match
        if (next_tok.token_type == tkNL || next_tok.token_type == tkUNDEF) {
          if (opcodes[opcounter].no_arg == true) {
            write_byte(opcodes[opcounter].no_arg_b);
            continue;
          } else {
            cerr << "error: opcode " << opcodes[opcounter].name << " expects args\n";
            return fail;
          }
        } else if (next_tok.token_i[0] == '#') {
          // it's a literal!
          next_tok.token_i.erase(0, 1);
          if (opcodes[opcounter].lit == true) {
            if (parse_num(next_tok.token_i) < 256) {
              // in range
              write_byte(opcodes[opcounter].lit_b);
              write_byte(parse_num(next_tok.token_i));
              pass_counter++; continue;
            } else {
              cerr << "error: opcode " << opcodes[opcounter].name << " takes only 8-bit literals\n";
              return fail;
            }
          } else {
            // this thing doesn't like literals!?
            cerr << "error: opcode " << opcodes[opcounter].name << " doesn't take literals\n";
            return fail;
          }
        } else if (next_tok.token_type == tkNUM) {
          // it's a number!
          // this could go multiple ways.
          
          // is it relative?
          if (opcodes[opcounter].relative == true) {
            // it is!
            unsigned char relative_temp;
            if (next_tok.token_i[0] == '-') {
              relative_temp = 256-parse_num(next_tok.token_i);
            } else {
              relative_temp = parse_num(next_tok.token_i);
            }
            
            write_byte(opcodes[opcounter].relative_b);
            write_byte(relative_temp);
            pass_counter++; continue;
          }
          
          if (parse_num(next_tok.token_i) < 256) {
            // 8-bit!
            if (opcodes[opcounter].one8 == true) {
              write_byte(opcodes[opcounter].one8_b);
              write_byte(parse_num(next_tok.token_i));
              pass_counter++; continue;
            } else {
              cerr << "error: opcode " << opcodes[opcounter].name << " can't take 8-bit args\n";
              return fail; 
            }
          } else if (parse_num(next_tok.token_i) < 65536) {
            // 16-bit!
            if (opcodes[opcounter].one16 == true) {
              write_byte(opcodes[opcounter].one16_b);
              write_byte(parse_num(next_tok.token_i) & 0xFF);
              write_byte(parse_num(next_tok.token_i) >> 8);
              pass_counter++; continue;
            } else {
              cerr << "error: opcode " << opcodes[opcounter].name << " can't take 16-bit args\n";
              return fail; 
            }
          } else if (parse_num(next_tok.token_i) < 0xFFFFFF+1) {
            // 24-bit!
            if (opcodes[opcounter].one24 == true) {
              write_byte(opcodes[opcounter].one24_b);
              write_byte(parse_num(next_tok.token_i) & 0xFF);
              write_byte((parse_num(next_tok.token_i) >> 8) & 0xFF); // crazy middle byte
              write_byte(parse_num(next_tok.token_i) >> 16);
              pass_counter++; continue;
            } else {
              cerr << "error: opcode " << opcodes[opcounter].name << " can't take 24-bit args\n";
              return fail; 
            }
          }
        }
      }
      
      if (!match_count) {
        // no matches
        cerr << "error: unknown opcode " << tokens[pass_counter].token_i << "\n";
        return fail;
      }
    } else if (tokens[pass_counter].token_type == tkLB) {
      tokens[pass_counter].token_i.pop_back();
      new_label(tokens[pass_counter].token_i, org);
    } else if (tokens[pass_counter].token_type == tkNL) {
      continue;
    } else {
      // probably a blank line
      continue;
    }
  }
  
  return success;
}


void raw_data(int range, string name)
{
  while (hint_next_token().token_type != tkNL &&
         hint_next_token().token_type != tkUNDEF) {
    if (hint_next_token().token_type == tkCOMMA) {
      pass_counter++; continue;
    }
    
    directive_arg(name, range);
    
    if (range == r8) {
      write_byte(tr & 0xFF);
      return;
    } else if (range == r16) {
      write_byte(tr >> 8);
      write_byte(tr & 0xFF);
    }
  }
}


token hint_next_token()
{
  if (pass_counter >= tokens.size()) {
    cerr << "error: args expected\n";
    exit(fail);
  }
  
  return tokens[pass_counter+1];
}


void directive_arg(string str, int range)
{
  unsigned int label_search;
  int label_count = 0;
  
  pass_counter++;
  
  if (tokens[pass_counter].token_type == tkOP) { // if it's not number
    for (label_search = 0; label_search < labels.size(); label_search++) { // look for a label!
      if (labels[label_search].name == tokens[pass_counter].token_i) {
        tr = labels[label_search].val; // it's a match!
        label_count++; // no need for unsolved.
        break;
      }
    }
    
    if (!label_count) { // if no match was found
      // time for an unsolved!
      new_unsolved(tokens[pass_counter].token_i, org, range-1);
      return;
    }
  } else if (tokens[pass_counter].token_type == tkNUM) {
    tr = parse_num(tokens[pass_counter].token_i);
  } else {
    cerr << "error: " << str << " expects args\n";
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
  
  // ...well, that's before bug checks came in
  if (org > (base*(cur_bank+2))) {
    if (cur_bank >= rombanks) {
      cerr << "error: no more room in rom\n";
      exit(fail);
    } else {
      cerr << "warning: bank overflow; rolling into bank " << cur_bank+1 << "\n";
      cerr << "org: " << org << " base: " << base << " banksize: " << banksize << "\n";
      cur_bank++;
    }
  }
}


void new_label(string name, long val)
{
  labels.push_back({name, val});
}


void new_unsolved(string name, long loc, int type)
{
  unsolveds.push_back({name, loc, type, false, 0, 0, 0, 0});
}


void new_unsolved_opcode(string name, long loc, int type,
             unsigned char byte, unsigned char b8, unsigned char b16, unsigned char b24)
{
  unsolveds.push_back({name, loc, type, true, byte, b8, b16, b24});
  return;
}


int solve_unsolveds()
{
  unsigned int label_search = 0;
  int match_num = 0;
  
  for (unsigned int counter = 0; counter < unsolveds.size(); counter++) {
    match_num = 0;
    
    for (label_search = 0; label_search < labels.size(); label_search++) {
      if (labels[label_search].name == unsolveds[counter].name) {
        // a match!
        match_num++;
        if (unsolveds[counter].type == tDB) {
          if (labels[label_search].val > 0xFF) {
            // too big!
            cerr << "error: db requires 8-bit args\n";
            return fail;
          }
          
          org = unsolveds[counter].loc;
          write_byte(labels[label_search].val);
        } else if (unsolveds[counter].type == tDW) {
          if (labels[label_search].val > 0xFFFF) {
            // too big!
            cerr << "error: dw requires 16-bit args\n";
            return fail;
          }
          
          org = unsolveds[counter].loc;
          write_byte(labels[label_search].val >> 8);
          write_byte(labels[label_search].val & 0xFF);
        }
      }
    }
    
    if (!match_num) {
      cerr << "error: no such label " << unsolveds[counter].name << '\n';
      return fail;
    }
  }
  
  return success;
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
  
  
  if (compcheck_flag == true) {
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
}