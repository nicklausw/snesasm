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


// opcode struct
// only works with no-args
typedef struct {
  string name;
  bool no_arg; unsigned char no_arg_b; // xxx
  bool one8; unsigned char one8_b;     // xxx $00
  bool one16; unsigned char one16_b;   // xxx $1010
  bool one24; unsigned char one24_b;   // xxx $101010
  bool ind; unsigned char ind_b;       // xxx ($10)
  bool lit; unsigned char lit_b;       // xxx #$10
  bool x8; unsigned char x8_b;         // xxx $10,x
  bool x16; unsigned char x16_b;       // xxx $1010,x
  bool x24; unsigned char x24_b;       // xxx $101010,x
} opcode;


// we need to shorthand...a lot
#define t true
#define f false

// opcode list
opcode opcodes[] = {
  {"xce", t, 0xFB, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00},
  {"clc", t, 0x18, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00},
  {"dex", t, 0xCA, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00},
  {"adc", f, 0x00, t, 0x65, t, 0x6D, t, 0x6F, t, 0x72, t, 0x69, t, 0x63, t, 0x7D, t, 0x7F},
  {"rep", f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, t, 0xC2, f, 0x00, f, 0x00, f, 0x00},
  {"sep", f, 0x00, f, 0x00, f, 0x00, f, 0x00, f, 0x00, t, 0xE2, f, 0x00, f, 0x00, f, 0x00},
  {"ldx", f, 0x00, t, 0xA6, t, 0xAE, f, 0x00, f, 0x00, t, 0xA2, f, 0x00, f, 0x00, f, 0x00},
  {"lda", f, 0x00, t, 0xA5, t, 0xAD, t, 0xAF, t, 0xB2, t, 0xA9, f, 0xB5, f, 0xBD, f, 0xBF},
  {"sta", f, 0x00, t, 0x85, t, 0x8D, f, 0x8F, t, 0x92, f, 0x00, f, 0x95, f, 0x9D, f, 0x9F}
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
    } else if (isdigit(ins[counter]) || ins[counter] == '$' || ins[counter] == '%') {
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
      } else if (tokens[counter].token_i == "db") {
        while (hint_next_token(counter, "db").token_type != tkNL &&
             hint_next_token(counter, "db").token_type != tkUNDEF) {
          counter = numeric_arg("db", counter, r8); write_byte(tr & 0xFF);
        }
      } else if (tokens[counter].token_i == "dw") {
        while (hint_next_token(counter, "dw").token_type != tkNL &&
             hint_next_token(counter, "dw").token_type != tkUNDEF) {
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
    } else if (tokens[counter].token_type == tkNUM || tokens[counter].token_type == tkARG) {
      cerr << "error: loose phrase " << tokens[counter].token_i << "\n";
      return fail;
    } else if (tokens[counter].token_type == tkOP) {
      // opcode
      int match_count = 0;
      token next_tok = hint_next_token(counter, "arg check");
      
      for (unsigned int opcounter = 0; opcounter < sizeof(opcodes)/sizeof(opcode); opcounter++) {
        if (tokens[counter].token_i == opcodes[opcounter].name) {
          match_count++;
        } else continue;
        
        // it's a match
        if (next_tok.token_type == tkNL) {
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
              counter++; continue;
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
          // this could go multiple ways. for now we'll only
          // do the one-arg way.
          
          if (parse_num(next_tok.token_i) < 256) {
            // 8-bit!
            if (opcodes[opcounter].one8 == true) {
              write_byte(opcodes[opcounter].x8_b);
              write_byte(parse_num(next_tok.token_i));
              counter++; continue;
            } else {
              cerr << "error: opcode " << opcodes[opcounter].name << " can't take 8-bit args\n";
              return fail; 
            }
          } else if (parse_num(next_tok.token_i) < 65536) {
            // 16-bit!
            if (opcodes[opcounter].one16 == true) {
              write_byte(opcodes[opcounter].x16_b);
              write_byte(parse_num(next_tok.token_i) & 0xFF);
              write_byte(parse_num(next_tok.token_i) >> 8);
              counter++; continue;
            } else {
              cerr << "error: opcode " << opcodes[opcounter].name << " can't take 16-bit args\n";
              return fail; 
            }
          } else if (parse_num(next_tok.token_i) < 0xFFFFFF+1) {
            // 24-bit!
            if (opcodes[opcounter].one24 == true) {
              write_byte(opcodes[opcounter].x24_b);
              write_byte(parse_num(next_tok.token_i) & 0xFF);
              write_byte((parse_num(next_tok.token_i) >> 8) & 0xFF); // crazy middle byte
              write_byte(parse_num(next_tok.token_i) >> 16);
              counter++; continue;
            } else {
              cerr << "error: opcode " << opcodes[opcounter].name << " can't take 24-bit args\n";
              return fail; 
            }
          }
        }
      }
      
      if (!match_count) {
        // no matches
        cerr << "error: unknown opcode " << tokens[counter].token_i << "\n";
        return fail;
      }
    } else if (tokens[counter].token_type == tkLB) {
      tokens[counter].token_i.pop_back();
      new_label(tokens[counter].token_i, org);
    } else if (tokens[counter].token_type == tkNL) {
      continue;
    } else {
      // probably a blank line
      continue;
    }
  }
  
  return success;
}


token hint_next_token(unsigned int counter, string cur)
{
  if (counter >= tokens.size()) {
    cerr << "error: " << cur << " expects args\n";
    exit(fail);
  }
  
  return tokens[counter+1];
}


int numeric_arg(string str, unsigned int counter, int range)
{
  unsigned int label_search;
  int label_count = 0;
  
  counter++;
  
  if (tokens[counter].token_type == tkOP) { // if it's not number
    for (label_search = 0; label_search < labels.size(); label_search++) { // look for a label!
      if (labels[label_search].name == tokens[counter].token_i) {
        tr = labels[label_search].val; // it's a match!
        label_count++; // no need for unsolved.
        break;
      }
    }
    
    if (!label_count) { // if no match was found
      // time for an unsolved!
      new_unsolved(tokens[counter].token_i, org, range-1);
      return counter;
    }
  } else if (tokens[counter].token_type == tkNUM) {
    tr = parse_num(tokens[counter].token_i);
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