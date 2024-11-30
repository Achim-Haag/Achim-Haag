/*
    14.11.24/AH
    My first C program, derived from c't 25/2024 (8.11.24), page 66 (https://github.com/607011/wordle-c)
    I will type this page's contents step by step, amended by my own modifications
    Insert:
        our brain EPROM's programming durability depends on how we program it:
        listen -> read -> keyboard input -> handwriting

    22.11.24/AH
    After typing in some more of the original source code, I found hints relating curly braces a.s.o :
    https://www.gnu.org/prep/standards/standards.html
    So I decided to vary my preferred method (from other programming languages)
    and set the braces in one line with the corresponding statement for functions, struct and enum.
    Another point is writing control statements without curly braces.
    I tend to share the opinion: always use braces as indention isn't recognized by the compiler
    (e.g. adding statements without braces - although indented - lead to misbehaviour and misunderstanding)

    28.11.24/AH
    Completed source, tested ANSI coloring in MVSC / Windows environment
    I will add compiler option /FA to .vscode\tasks.json - it gives a listing for an old fart like me.
    /FA knows optional arguments: c = machine code, s = source code, u = Listing in UTF-8 instead of ANSI

    30.11.24/AH
    Someone ("Hornschorsch") reworked the whole story, now I have to change my copy
    Although I understand the modifications: number of words in word list not dependend on constant
*/

/*
    Activate the following statement for enhanced output
    (Later, I will change it to a program parameter -v (verbose) )
*/
#define MYDEBUG

/*
  Program logic:
    * main:
      count words in word list and select word to guess
      loop MAX_TRIES times
        * get_input:
          until user entered 5 characters and word is in wordlist
            read user's input characters
            * word_is_allowed:
              check if word is in wordlist
        * update_state:
          mark correct and misplaced characters
          save the result in structure
            * is_character_unmarked
              check word for guessed character
        * print_result:
          show result and wrong characters to user
        compare word with word from wordlist
          show final result
          * another_round
            ask user for another round
*/

/*
    Declares the Windows Header file if Microsoft Visual-C (MSVC) is used for compile
    Needed for activation of ANSI coloring in cmd.exe virtual terminal
*/
#if _MSC_VER          // see https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170
#include <Windows.h>
#include <wchar.h>
#endif

// Declaration of external library functions used in this program
// Libraries surrounded by "<>" are searched in the compiler's default path
#include <ctype.h>      // tolower()
#include <stdbool.h>    // bool, true and false 
#include <stdio.h>      // stdin, getchar(), fgets()
#include <stdlib.h>     // atoi(), srand(), rand()
#include <string.h>     // strncmp(), strchr() 
#include <time.h>       // time()

// Declarations contained in separate file words.h in the same directory
#include "words.h"      // structure with data used by this program, must be defined locally - later !

// Declaration of precompiler variables
#define WORD_BUF_LEN (WORD_LENGTH + 1)
#define MAX_TRIES (6)

// this defines status as an enumeration template
// it consists of a set of elements that are assigned to numbers
// so it's easier to understand the meaning of "blabla = UNMARKED" in opposite to "blabla = 0"
// it doesn't need any storage at runtime as it's only a logical definition
enum status
{
  UNMARKED,       // 0 (here implicit assigned by order, possible also explicit assignment: e.g.UNMARKED=0)
  NOT_PRESENT,    // 1
  PRESENT,        // 2
  CORRECT         // 3
};

/*
    this defines state_t as an enumeration of the above enumeration status
    At this point, we even don't need any RAM as it's just a description for the compiler
*/
typedef enum status state_t;

/*
    A structure is some kind of overlay for an area of memory
    (although the typedef doesn't reserve RAM at this moment - this memory has to be instanciated, see later)
    This structure contains four variables: a pointer, a string, an enumeration array and an integer
*/
typedef struct
{
    // pointer to word in wordlist (asterisk defines address pointer)
  const char* word;
    // word to guess
  char guess[WORD_BUF_LEN];
/*
    highlights found characters
    state_t is an enumeration and defined indirect (see above):
    1. typedef for state_t as enumeration status
    2. enumeration named status with four possible values
*/    
  state_t result[WORD_LENGTH];
    // mark characters guessed by user and are in word from wordlisg
  bool used[WORD_LENGTH];
    // Counts our guesses
  int n_tries;

} game_state;

/*
  Subfunction word_is_allowed
    checks the input if it's in the wordlist
    After the for loop breaks something (return stack ?), I changed the two returns
    to a boolean variable and a break for easier debugging what happened
*/
bool word_is_allowed(const char* word)
{
#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Entering subfunction\n", __func__, __LINE__);
#endif

  // Sequential search the guessed word in wordlist
  bool wordfound = false;    // Assume we will not find any character of user's input
  for (int counter = 0; (words[counter] != NULL); ++counter) {
#ifdef MYDEBUG
    printf("\t#DBG %s@%d # strncmp [%s] with [%s]\n", __func__, __LINE__, word, words[counter]);
#endif
    if (strncmp(word, words[counter], WORD_LENGTH) == 0) {
      wordfound = true;   // Wow, we have found a character in our word that user guessed !
#ifdef MYDEBUG
      printf("\t#DBG %s@%d # found word %s\n", __func__, __LINE__, words[counter]);
#endif
      break;  // exit for-loop immediately
    }
  }

  // Return the search result (true/false) to caller
#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Returning with '%s'\n", __func__, __LINE__, wordfound ? "true" : "false");
#endif
  return wordfound;
}

/*
  Subfunction is_character_unmarked
    Scan from wordlist for a character of user's guess
    If not marked as "given by user", return "true", else return "false"
    BTW: although a little bit more complex, I personally would prefer
      not exiting in the mid of a function if I can avoid it.
      So I will change it to a break out of the loop and an additional variable
*/
bool is_character_unmarked(game_state* state, char c)
{
#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Entering subfunction\n", __func__, __LINE__);
#endif

  bool charfound = false;    // Assume we will not find any character of user's input

  for (int counter = 0; counter < WORD_LENGTH; ++counter) {
    if ( (state->word[counter] == c) && (state->used[counter] == false) ) {
      state->used[counter] = true;
      charfound = true;   // Wow, we have found a character in our word that user guessed !
      break;  // exit for-loop immediately
    }
  }
  return charfound;     // exit subfunction with scan results
}

/*
  Subfunction update_state
  Process user's input
*/
void update_state(game_state *state)
{
#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Entering subfunction\n", __func__, __LINE__);
#endif
 
  // mark every character as "unmarked" before we process the input
  for (int counter = 0; counter < WORD_LENGTH; ++counter) {
    state->result[counter] = UNMARKED;
    state->used[counter] = false;
  }

  // find correct characters and mark then
  // (Btw: I don't understand, why this and the upper loop aren't combined ?
  //      there's no reference to other array elements than "counter" ?!?,
  //      maybe later combine them ?)
  for (int counter = 0; counter < WORD_LENGTH; ++counter) {
    if (state->guess[counter] == state->word[counter]) {
      state->result[counter] = CORRECT;
      state->used[counter] = true;
    }
  }

  // Now process every character that's there but not on the right position
  for (int counter = 0; counter < WORD_LENGTH; ++counter) {
    // if character is marked CORRECT, skip it
    if (state->result[counter] == CORRECT) {
      continue;    // next for-loop
    }
                            // call subfunction in this source and set result
                            // (PRESENT/NOT_PRESENT) depending on bool return of subfunction
    state->result[counter] = is_character_unmarked(state, state->guess[counter])
      ? PRESENT
      : NOT_PRESENT;
  }
}

/*
  Subfunction get_input
    Asking user for his guess and processing his input
*/
void get_input(game_state* state)
{

#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Entering subfunction\n", __func__, __LINE__);
#endif

  // loop until user input ok
  bool bad_word;
  do {      // while (bad_word)
    printf("\n%d. trial: ", state->n_tries);
    // read input from console (WORD_LENGTH characters)
    bad_word = false;
    for (int charin = 0; charin < WORD_LENGTH; charin++) {
      state->guess[charin] = getchar();         // read character from console
      if (state->guess[charin] == '/n') {
        state->guess[charin] = '\0';
        bad_word = true;
        break;    // exit for-loop
      } 
    }
    // read (and drop) remaining characters (after the WORD_LENGTH one) using a while-loop
    if (!bad_word) {
      while (getchar() != '\n') {};
    }

    // set end of string to char after WORD_LENGTH
    state->guess[WORD_LENGTH] = '\0';

#ifdef MYDEBUG
    printf("\t#DBG %s@%d # Input: [%s]\n", __func__, __LINE__, state->guess);
#endif

    // process incorrect user input
    if (bad_word) {
      printf("Please enter exactly (!) %d characters !\n", WORD_LENGTH);
    }
    else {
/*    
  Uncommented the following two statements as one gets only character hints guessing a word from the wordlist
  So if one doesn't know the wordlist, guessing is nearly impossible
	    bad_word = !word_is_allowed(state->guess);
	    if (bad_word)
*/      

#ifdef MYDEBUG
      printf("\t#DBG %s@%d # Back from word_is_allowed\n", __func__, __LINE__);
#endif
      if (!word_is_allowed(state->guess)) {
        printf("Word not found in my wordlist\n");
      }
      else {
        printf("Word found in my wordlist\n");
      }

    }
#ifdef MYDEBUG
    printf("\t#DBG %s@%d # badword is  %s\n", __func__, __LINE__, bad_word ? "true" : "false");
#endif
  } while (bad_word) ;   // end "do ... while" loop

#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Leaving function\n", __func__, __LINE__);
#endif
}

// I pasted the c't original source of get_input to check for an input error I thought, I made
// (by renaming this and my functions name, I could easily change between them).
// But as the behaviour is the same, I found the culprit in the last "if" block
// as it sets bad_word to "!word_is_allowed", so if the guessed word isn't in the list,
// the do-while will never be exited and no character hint will be shown.
/***************** c't Github Original Start ************************************/
void get_input_from_github(game_state* state)
{
    // solange eine Eingabe anfordern, bis sie gültig ist
    bool bad_word;
    do
    {
        printf("\n%d. Versuch: ", state->n_tries);
        // Eingabe lesen
        bad_word = false;
        for (int i = 0; i < WORD_LENGTH; i++) {
            state->guess[i] = getchar();
            if (state->guess[i] == '\n') {
                state->guess[i] = '\0';
                bad_word = true;
                break;
            }
        }
        // überflüssige Zeichen verwerfen
        if (!bad_word)
            while (getchar() != '\n')
                ;
        // nach dem 5. Zeichen abschneiden
        state->guess[WORD_LENGTH] = '\0';
#ifdef DEBUG
        printf("Eingabe: '%s'\n", state->guess);
#endif
        if (bad_word) {
            printf("Bitte %d Buchstaben eingeben.\n", WORD_LENGTH);
	} else {

/*    
	    bad_word = !word_is_allowed(state->guess);
	    if (bad_word)
*/      
      if (!word_is_allowed(state->guess))
		    printf("Das Wort ist nicht in der Liste erlaubter Wörter.\n");
	}

printf("Bottom of while-loop, bad_word is %d\n", bad_word);

    }
    while (bad_word);
printf("Exit while-loop, bad_word is %d\n", bad_word);
}


/****************** c't Github Original Ende   ************************************/

/*
  Subfunction print_result
  Give feedback/hints about the guess
*/
void print_result(const game_state* state)
{

#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Entering subfunction\n", __func__, __LINE__);
#endif
  // Show result in a nice way (ANSI escape sequences for coloring)
  // Explanation see http://jafrog.com/2013/11/23/colors-in-terminal.html
  // or https://ss64.com/nt/syntax-ansi.html
  // Hint: \033 (3*8+3=27) is an octal representation of 0x1B (dec 27 = ESC)
  printf("! ");

  for (int counter = 0; counter < WORD_LENGTH; ++counter) {
    switch (state->result[counter]) {
      case CORRECT:
        // Characters in the right position marked with green background
        printf("\033[37;42;1m");  // ESC writen as octal number (3*8+3 = 27)
        break; // break ends this switch-part
      case PRESENT:
        // Characters in word but wrong positioned marked with yellow background
        printf("\x1b[37;43;1m");   // ESC writen as hexadecimal number (1*16 + 11 = 27)
        break; // break ends this switch-part
      case NOT_PRESENT:
        // Characters not in word not marked at all, but:
        // since there is no "break"-Statement, they are processd
        // by the following "default:" branch
        // In short: "case NOT_PRESENT" and "default:" refers to the same code
      default:    // the "else" branch of this switch-statement
        // Characters not in word marked with red background
        // Characters not in word marked with red background
        printf("\033[37;41;1m");
        break; // break ends this switch-part
    }
    printf("%c", state->guess[counter]);
  }
  // Reset font and background colors to their defaults
  printf("\033[0m\n");
}

bool another_round(void)
{
#ifdef MYDEBUG
  printf("\t#DBG %s@%d # Entering subfunction\n", __func__, __LINE__);
#endif

  printf("Another round ? [j/n] ");
  char answer = (char)tolower(getchar()) ; // read pressed key from keyboard
  // drop superfluous characters
  if (answer != '\n') {
    while (getchar() != '\n') ;
  }
  bool yes = ((answer == 'j') || (answer == '\n')) ;
  if (yes) {
    printf("\nOK, now go ahead...\n");
  }
  return yes;
}

/*
  Main program
*/
int main(int argc, char* argv[])
{


/*
  To be sure, I run the least compiled .exe:
  #pragma message: print filename (source) and  compile/build date/time while compiling/building and
  printf : print filename (.exe) and  compile/build date/time when running
*/
#pragma message ("***** Build " __FILE__ " at " __DATE__ " " __TIME__ "*****")   
  printf("***** Running %s,\nBinary build date: %s @ %s *****\n\n", argv[0], __DATE__, __TIME__);

/*
  To enable ANSI text formatting in Windows cmd.exe, I had to add some extra code in my environment (W10 22H2)
  Stolen from https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example-of-select-anniversary-update-features
  Chapter "Samples" - "Example of SGR terminal sequences"
*/

#if _MSC_VER          // Necessary only in Windows environment as bash should know ANSI by default

/*
  Enable Windows 10 cmd.exe ANSI processing
*/
  // Set output mode to handle virtual terminal sequences
  DWORD LastError = 0;          // Keep GetLastError in own variable to not interfere with printf
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE)   {
    LastError = GetLastError();
    printf("Cannot get handle for standard device (STD_OUTPUT_HANDLE), GetStdHandle RC=%d", LastError);
    return LastError;
  }

  DWORD dwMode = 0;
  if (!GetConsoleMode(hOut, &dwMode)) {
    LastError = GetLastError();
    printf ("Cannot get console mode, GetConsoleMode RC=%d", LastError);
    return LastError;
  }

  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(hOut, dwMode)) {
    LastError = GetLastError();
    printf("Cannot set console mode to virt. terminal proc., SetConsoleMode RC=%d", LastError);
    return LastError;
  }

/*    In sample, but unused here

    // Try some Set Graphics Rendition (SGR) terminal escape sequences
    wprintf(L"\x1b[31mThis text has a red foreground using SGR.31.\r\n");
    wprintf(L"\x1b[1mThis text has a bright (bold) red foreground using SGR.1 to affect the previous color setting.\r\n");
    wprintf(L"\x1b[mThis text has returned to default colors using SGR.0 implicitly.\r\n");
    wprintf(L"\x1b[34;46mThis text shows the foreground and background change at the same time.\r\n");
    wprintf(L"\x1b[0mThis text has returned to default colors using SGR.0 explicitly.\r\n");
    wprintf(L"\x1b[31;32;33;34;35;36;101;102;103;104;105;106;107mThis text attempts to apply many colors in the same command. Note the colors are applied from left to right so only the right-most option of foreground cyan (SGR.36) and background bright white (SGR.107) is effective.\r\n");
    wprintf(L"\x1b[39mThis text has restored the foreground color only.\r\n");
    wprintf(L"\x1b[49mThis text has restored the background color only.\r\n");

    return 0;
*/

#endif    // End Windows ANSI enabling section 


/************************* TEST END *************************** */

/*
    The following "ternary operator" replaces an if/then/else clause.
    Question mark is "then", colon is "else".
    If the number of parameters given to this program (including argument 1 = Path+Filename of this program)
    is greater than 1, an argument is given explicitly and determins the "random seed".
    This eases testing as for a specific initial random seed the random generator returns the same series of numbers
    If no explicit parameter is specified, the actual timestamp is used for the initialization of the random generater,
    leading to nonpredictable random number series.
*/
  unsigned int seed = (argc > 1)
                      ? (unsigned int)atoi(argv[1])
                      : (unsigned int)time(NULL) ;

  printf("Initial random generator seed: %d\n", seed);

/*
    Prepare the random number generation for later use of the "rand" function
*/
  srand(seed);

  printf("\nNERD WORD\n\n"
         "Guess the word with %d characters in no more than %d trials.\n"
         "(Abort = Ctrl+C)\n",
         WORD_LENGTH, MAX_TRIES);

/*
    We run this program until the user aborts.
    we cannot see at this moment, how this decision is made, but we know, 
    it must made by setting keepRunning to it's logical value "false".
*/
  bool keepRunning = true;
  while (keepRunning) {

#ifdef MYDEBUG
    printf("\t#DBG %s@%d # Entering while-loop, keepRunning is %s\n", __func__, __LINE__, keepRunning ? "true" : "false");
#endif


/*
    First of all create a "real instance" of type game_state, it's name is "state"
    It's a structure combining multiple elements in storage (here we need our RAM ;-)
    The layout of the structure is defined above as a typeset "game_state"
    so "game_state" is - from a higher point of view - the same as an integer, character, string, whatever
    But...as far as I think, it's no "executable" statement, it just reserves memory in it's scope
    (the compiler generates code to allocate RAM)
    Btw: scope means here: "main"
*/
#ifdef MYDEBUG
    printf("\t#DBG %s@%d # Calling game_state\n", __func__, __LINE__);
#endif
    game_state state;

/*
    Count all words in the wordlist (until we reach a null pointer element)
*/
    int num_words;
    for (num_words = 0; words[num_words] != NULL; num_words++) {};  // One-line loop

#ifdef MYDEBUG
    printf("\t#DBG %s@%d # Table word count is %d\n", __func__, __LINE__, num_words);
#endif

/*
    Now we fill one variable - the pointer to the word - with the address of a randomly selected
    word of our wordlist (that comes out of words.c which is linked together with this program)
    Important: as the word in array "words" is addressed by modulo (%) NUM_WORDS,
    NUM_WORDS must not be higher than the real number of words in the array "words",
    else something unpredictable would occur
    (maybe NUM_WORDS should be defined near "words" if this is possible ?)
    Debugging: the other variables in structure "state" are undefined at the first entry into the loop
*/
    state.word = words[rand() % num_words];

/*
    only for testing: show me the selected word
    #-marked statements are processed by the compilers preprocessor
    (personally, I call it precompiler in memoriam to PL/I ;-)
    To activate this printf, a compiler option "-D DEBUG" must be given that defines DEBUG (no value needed)
    Not to be confused with _DEBUG (underscore !) that is defined by MSVC by the /MTd or /MDd option,
    see https://learn.microsoft.com/en-us/cpp/c-runtime-library/debug?view=msvc-170
    Changed to my own precompiler variable MYDEBUG for simplicity
*/
#ifdef MYDEBUG
    printf("\t#DBG %s@%d # Hint: %s\n", __func__, __LINE__, state.word);
#endif

/*
  Now we run another loop that asks our guesses of the characters
  The for loop counts a number variable from a starting point to an ending point
  In the C language, the ending point doesn't has to be dependend on the number variable alone,
  it can be extended by other logical expressions
*/
    bool doRestart = false;
    for (state.n_tries = 1; 
         state.n_tries <= MAX_TRIES && !doRestart;
         ++state.n_tries)    {
#ifdef MYDEBUG
      printf("\t#DBG %s@%d # Entering for-loop, state.n_tries is %d\n", __func__, __LINE__, state.n_tries);
#endif
    
/*
    we call the above defined function to get user's input.
    The "&" means: give the address of variable (structure) "state" to this function,
    so this function is able to modify this structure - i.e. the structure's variables
*/   

    // ask user for keyboard input
#ifdef MYDEBUG
      printf("\t#DBG %s@%d # Calling get_input\n", __func__, __LINE__);
#endif
      get_input(&state);

    // process user's input
#ifdef MYDEBUG
      printf("\t#DBG %s@%d # Calling update_state\n", __func__, __LINE__);
#endif
      update_state(&state);

    // show results
      print_result(&state);

    // Compare input word with word to guess, if equal, user wins
      if (strncmp(state.guess, state.word, WORD_LENGTH) == 0) {
        printf ("\nYee-haw, you've won after %d. trials!\n", state.n_tries);
        doRestart = true;
        keepRunning = another_round();
      }
      else {
        if (state.n_tries == MAX_TRIES) {
          printf("You don't guess the word, it was %s.\n", state.word);
          keepRunning = another_round();
        }
      }

    } // end "for num_words" loop
#ifdef MYDEBUG
      printf("\t#DBG %s@%d # Bottom of while-loop, keepRunning is %s\n", __func__, __LINE__, keepRunning ? "true" : "false");
#endif
  } // end "while (keepRunning)" loop
  printf("\nWaiting for you debugging me,\nplease press Enter after debugging has ended\n");
  return EXIT_SUCCESS;
}