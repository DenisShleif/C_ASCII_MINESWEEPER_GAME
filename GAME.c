///-------------------------/////////////////-----------------------------///
///                         /////////////////                             ///
///                         ///PRE PROGRAM///                             ///
///                         /////////////////                             ///
///-------------------------/////////////////-----------------------------///


///--------------------------------------------------------///
///                        LIBRARIES                       ///
///--------------------------------------------------------///

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <errno.h>

///--------------------------------------------------------///
///                       CONSTANTS                        ///
///--------------------------------------------------------///

///BoardSize
#define MAX_SIZE_X 24
#define MAX_SIZE_Y 16
#define MIN_SIZE_X 9
#define MIN_SIZE_Y 9

///Game Constatants
#define MENU_DELAY 3
#define MENU_OPTIONS 4
#define GAME_OPTIONS 5
#define SAVE_OPTIONS 5
#define INSTRUCTION_OPTIONS 2
#define PAUSE_MENU_OPTIONS 5
#define MINE -1
#define MIN_MINES 10

///String
#define MAX_LENGTH 80
#define MIN_CHAR '0'
#define MAX_CHAR '9'

///Parsing
#define BASE 10
#define MAX_DIGITS 2

///Debuging
#define DEBUG 0

///Screen
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

///File I/O
#define BASE_FILE "SAVE/ORIGIN.dat"
#define FORMAT ".dat\0"
#define FOLDER "SAVE/\0"
#define MAX_FILES 10



///--------------------------------------------------------///
///                   ENUMERATED TYPES                     ///
///--------------------------------------------------------///

enum squareStatusType {nothing = 0, dug = 1, question = 2, flag = 3};
enum boolean {FALSE = 0, TRUE = 1};
enum gameStatusType {notPlaying = 0, play = 1,minesCreated = 2, lose = 3, win = 4};
enum saveLoadGameType {limited = 0, unlimited = 1};

///--------------------------------------------------------///
///                       STRUCTURES                       ///
///--------------------------------------------------------///

struct squareType{
    int mineCount; ///The number of mines surrounding the square. -1 if the square itself contains a mine
    enum squareStatusType status; ///Stores the status of the square, whether it has just been initialized, dug, contains a queation mark or a flag
};

struct boardType{
    int width; ///The width of the board
    int height; ///The height of the board
    int numMine; ///The number of mines in the board
    int flagsUsed; ///The number of flags currently on the board
    int numCorrectFlag; ///The number of these flags that are actually on a mine
    int squaresLeft; ///The number of undug squares
    struct squareType square [MAX_SIZE_X][MAX_SIZE_Y]; ///Stores all of the information for each sqaure
};

struct fileType{
    char name [MAX_LENGTH];
};

struct baseFileType{
    int gamesWon;
    int gamesLost;
    int numFiles;
    struct fileType file [MAX_FILES];
};

///-------------------------/////////////////-----------------------------///
///                         /////////////////                             ///
///                         ////FUNCTIONS////                             ///
///                         /////////////////                             ///
///-------------------------/////////////////-----------------------------///

///--------------------------------------------------------///
///                     DECLARATIONS                       ///
///--------------------------------------------------------///

void outputGameOptions (enum gameStatusType status);
void outputMenu (enum gameStatusType status,struct baseFileType baseFile);
void outputBoard (struct boardType board, enum gameStatusType status);
void outputPauseMenu ();
void outputInstructionsPage (int page);
void outputSaveMenu (struct baseFileType baseFile);
void outputSaveOptions ();
void instructions();

///--------------------------------------------------------///
///                  GENERAL FUNCTIONS                     ///
///--------------------------------------------------------///

///Pauses the game until a key is struct
void pause (){
    char c;
    scanf("%c",&c);
}

///creates a random integer between min and max inclusively
int rb (int min, int max){
    return rand() % (max - min + 1) + min;
}

///Returns 1 if n is between min and max inclusively
int valBet (int n, int min, int max){
    return n >= min && n <= max;
}

///Delays the program
void delay (int milliseconds){
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}

///Prints out all of the variables
void debuger (struct boardType board, enum gameStatusType status){
    printf ("board.width = %i\n", board.width);
    printf ("board.height = %i\n", board.height);
    printf ("board.numMine = %i\n", board.numMine);
    printf ("board.flagsUsed = %i\n", board.flagsUsed);
    printf ("board.numCorrectFlag = %i\n",board.numCorrectFlag);
    printf ("board.squaresLeft = %i\n",board.squaresLeft);
    printf ("status = %i (notPlaying = 0, play = 1,minesCreated = 2, lose = 3, win = 4)\n\n",status);
}
///--------------------------------------------------------///
///                 General Input/Output                   ///
///--------------------------------------------------------///

///prints out the string like a typewriter
///wait is the delay between each letter
int animateText (const char* s, int wait){
    int i = 0;
    while (s [i]){
        printf ("%c",s[i++]);
        delay (wait);
    }
}

///Prints out the character c, n times
void repeatPrint (char c, int n){
    int i;
    for (i = 0; i < n; i++)
        printf ("%c",c);
}

///Parses an integer. Modified to work as a sub function for length calc
int simpleParseInt (char *s, int *i){
    int tmp = 0;

    while (valBet (s [*i],MIN_CHAR,MAX_CHAR)){
        tmp  *= BASE;
        tmp  += s [*i] - MIN_CHAR;
        (*i)++;
    }

    if (tmp == 0)
        tmp++;

    return tmp;
}

int positive (int n){
    if (n < 0)
        return 0;
    else
        return n;
}

///Advanced length calculation. Returns the lengths of a string (containing format specifiers) before it is printed
int lengthCalc (char *string, va_list listPointer){
    int i = 0;
    int length = 0;
    int spaces;

    ///Replacing the string with the variable arguments
    while (string [i]){
        if (string [i] == '%'){
            i++;
            ///Adjusts length based on spacing
            spaces = simpleParseInt(string,&i);
            length += spaces;

            ///Adjusts length based on the actual format specifier
            switch (string [i]){
                case 'c':
                    break;
                case 'f':
                    length += 6 + positive (log(va_arg(listPointer, int)) - spaces);
                case 'i':
                    length += positive (log(va_arg(listPointer, int)) - spaces) + 1;
                    break;
            }
        }
        else if (string [i] == 92){} ///Backslash
        else
            length++;
        i++;
    }
    return length;
}

///Prints a string alligned either left, centered or right at a certain column (place)
int placeString (int mode,int place, char *string, ...){
    int length;
    va_list listPointer; ///Declaring the list pointer
    va_start(listPointer, string); ///Setting the list pointer
    length = lengthCalc(string,listPointer); ///Calculates length
    va_start(listPointer, string); ///Reinitializes
    ///Print Spaces
    switch (mode){
        case 1: ///Alligned left
            repeatPrint(' ',place);
            break;
        case 2: ///Aligned center
            repeatPrint(' ',place - length / 2);
            break;
        case 3: /// aligned right
            repeatPrint(' ',place - length);
            break;
    }
    vfprintf (stderr,string,listPointer); ///Print the String
    va_end( listPointer); ///Ends the list
}

///Will input an integer between min and max
///will output functions
///will output strings with format specifiers
int input (int functionMode, int inputType,int inputFormat,int printStringMode,...){
    struct boardType board;
    struct baseFileType baseFile;
    enum gameStatusType status;
    int min, max, store,page;
    char *inputedString, *printedString;

    va_list listPointer; ///Declaring the list pointer
    va_start(listPointer, printStringMode); ///Setting the list pointer

    /** Variable arguments order :
    struct boardType board, enum gameStatusType status, int gamesWon, int gamesLost,int min, int max, char *inputedString, char *printedString, ... (format specifiers, only %c and %i are supported).
    Only put as an argument, if the modes require it.
    **/
        switch (functionMode){
         ///case 0: /// no functions called
            case 1: /// outputMenu called
                status = va_arg(listPointer, enum gameStatusType);
                baseFile = va_arg(listPointer, struct baseFileType);
                break;
            case 2: /// outputBoard called
                board = va_arg (listPointer, struct boardType);
                status = va_arg(listPointer, enum gameStatusType);
                break;
            case 3: /// outputBoard and outputGameOptions called
                board = va_arg (listPointer, struct boardType);
                status = va_arg(listPointer, enum gameStatusType);
                break;
            case 4: ///outputPauseMenu called
                break;
            case 5: ///outputInstructions called
                page = va_arg (listPointer, int);
                break;
            case 6: ///OutputSaveMenu Called
                baseFile = va_arg (listPointer,struct baseFileType);
                break;
            case 7: ///OutputSaveMenu Called and OutputSaveOptions
                baseFile = va_arg (listPointer,struct baseFileType);
                break;
        }
    if (inputType == 0){
        min = va_arg(listPointer, int); ///store min
        max = va_arg(listPointer, int); ///Stores max
    }
    else if (inputType == 1)
        inputedString = va_arg(listPointer, char*); ///Stores the address of the input string
    ///else if (inputType == 2)

    if (printStringMode > 0)
        printedString = va_arg(listPointer, char*); ///Stores the printed string
    do{
        ///Printing the functions
        system ("cls");
        switch (functionMode){
         ///case 0: /// no functions called
            case 1: /// outputMenu called
                outputMenu (status,baseFile);
                break;
            case 2: /// outputBoard called
                outputBoard (board,status);
                break;
            case 3: /// outputBoard and outputGameOptions called
                outputBoard (board,status);
                outputGameOptions (status);
                break;
            case 4:
                outputPauseMenu ();
                break;
            case 5:
                outputInstructionsPage(page);
                break;
            case 6:
                outputSaveMenu (baseFile);
                break;
            case 7:
                outputSaveMenu (baseFile);
                outputSaveOptions ();
                break;
        }

        ///String printing
        if (printStringMode >= 1){ ///Print string
            if (printStringMode >= 2){ /// Center Horrizontally
                if (printStringMode >= 3){///Center Vertically
                    repeatPrint ('\n',(SCREEN_HEIGHT - 2) / 2); ///Center Vertically
                }
                 repeatPrint(' ',(SCREEN_WIDTH - lengthCalc (printedString,listPointer)) / 2); ///Center Horizontally ///PROBLEM LINE///
            }
            vfprintf (stderr,printedString,listPointer); ///Print the String
        }

        ///Centers the input
        if (inputFormat >= 1){
            repeatPrint (' ',SCREEN_WIDTH/ 2); /// Center Horrizontally
                if (inputFormat >= 2){
                    repeatPrint ('\n',(SCREEN_HEIGHT - 2) / 2); ///Center Vertically
                }
        }
        ///Input
        fflush(stdin);
        if (inputType == 0)
            scanf("%i",&store);
        else if (inputType == 1)
            gets (inputedString);
    }while (inputType == 0 && !valBet (store,min,max));

    va_end( listPointer); ///Ends the list
    return store;
}


///--------------------------------------------------------///
///                    COORDINATE PARSE                    ///
///--------------------------------------------------------///

///Parses an integer
int parseInt (char *s, char term,int *i, int *store){
    int j = 0;
    *store = 0;

    ///Calculates the integer
    while (valBet (s [*i],MIN_CHAR,MAX_CHAR) && j < MAX_DIGITS){
        (*store) *= BASE;
        if (valBet (s [*i], MIN_CHAR, MAX_CHAR))
            (*store) += s [*i] - MIN_CHAR;
        else
            return FALSE;
        (*i)++;
        j++;
    }

    if (j == 0)
        return FALSE;

    ///Checks that for the terminating character
    if (s [*i] != term)
        return FALSE;

    (*i)++;

    return TRUE;
}

///PARSES A COORDINATE THE COORDINATE MUST BE IN INPUTED IN THIS FORMAT: (5,12)
int parseCoordinate (struct boardType board,char *s, int *x, int *y){
    int i = 0;
    if (s[i] != '(')
        return FALSE;
    i++;
    if (!parseInt (s,',',&i,x) || !parseInt (s,')',&i,y))
        return FALSE;
    if (s[i] != 0)
        return FALSE;
    if (!valBet (*x,1,board.width) || !valBet (*y,1,board.height))
        return FALSE;

    *y = board.height - *y;
    (*x)--;

    return TRUE;
}

///--------------------------------------------------------///
///                      FILE I/O                          ///
///--------------------------------------------------------///

void addtxt (char *s){
    char tmp [MAX_LENGTH];
    strcpy (tmp,s);
    strcpy (s,FOLDER);
    s += strlen (FOLDER);
    strcpy (s,tmp);
    s += strlen (tmp);
    strcpy (s,FORMAT);
}

void outputSaveMenu (struct baseFileType baseFile){
    FILE *file;
    int i,j;

    repeatPrint ('\n',3);
    placeString (2,SCREEN_WIDTH / 2,"SAVE AND LOAD GAME");
    repeatPrint ('\n',3);
    printf ("Save Files:\n");
    for (i = 0; i < baseFile.numFiles; i++){


        j = 0;
        printf ("%i: ",i + 1);
        while (baseFile.file[i].name[j] != '/')
            j++;
        j++;
        while (baseFile.file[i].name[j] != '.')
            printf ("%c",baseFile.file[i].name[j++]);
        printf ("\n");
    }
    if (i + 1 < MAX_FILES)
        printf ("%i: EMPTY\n",i + 1);
    printf ("\n");
}

void outputSaveOptions (){
    printf ("Options:\n");
    printf ("   Save Game: 1\tLoad Game: 2\tDelete Game: 3\tRename Game: 4\t   Exit: 5\n\n");
    placeString (2,SCREEN_WIDTH / 2,"Please Select an Option:");
    printf ("\n");
    repeatPrint (' ',SCREEN_WIDTH/2 - 1);
}

///------------Base File-----------///
void baseSaveFileUpdate (struct baseFileType baseFile){
    int i;
    FILE *file = fopen (BASE_FILE,"w");
    fprintf (file,"%i\n%i\n%i\n",baseFile.gamesLost, baseFile.gamesWon, baseFile.numFiles);
    for (i = 0; i < baseFile.numFiles; i++)
        fprintf (file,"%s\n",baseFile.file[i].name);
    fclose (file);
}

void baseSaveFileRead (struct baseFileType *baseFile){
    int i,tmp;
    FILE *baseFilePtr = fopen (BASE_FILE,"r");
    FILE *tmpFile;

    if (baseFilePtr == NULL){
        baseFile -> gamesLost = 0;
        baseFile -> gamesWon = 0;
        baseFile -> numFiles = 0;
    }
    else{
        fscanf (baseFilePtr,"%i\n%i\n%i\n",&(baseFile -> gamesLost),&(baseFile -> gamesWon),&(baseFile -> numFiles));
        for (i = 0; i < baseFile -> numFiles; i++){
            fscanf  (baseFilePtr,"%s\n",baseFile -> file[i].name);
            tmpFile = fopen (baseFile -> file[i].name,"r");
            if (tmpFile == NULL){
                i--;
                (baseFile -> numFiles)--;
            }
        }
    }
    fclose (baseFilePtr);
}

///------------Save File-----------///

void deleteGame (struct baseFileType *baseFile){
    int fileNum;
    if (baseFile -> numFiles > 1){
        fileNum = input (6,0,1,2,*baseFile,1,baseFile -> numFiles,"Please Select a file:\n");
        fileNum--;
    }
    else
        fileNum = 0;
    remove (baseFile -> file[fileNum].name);
    while (fileNum < baseFile -> numFiles - 1){
        strcpy (baseFile -> file[fileNum].name,baseFile -> file[fileNum + 1].name);
        fileNum++;
    }
    (baseFile -> numFiles)--;
}

void renameSave (struct baseFileType *baseFile){
    char temp [MAX_LENGTH];
    int fileNum;
    if (baseFile -> numFiles > 1){
        fileNum = input (6,0,1,2,*baseFile,1,baseFile -> numFiles,"Please Select a file:\n");
        fileNum--;
    }
    else
        fileNum = 0;
    input (6,1,1,2,*baseFile,temp,"Please enter a new name for your save file.\n");
    addtxt (temp);
    rename (baseFile -> file[fileNum].name, temp);
    strcpy (baseFile -> file[fileNum].name, temp);
}

void loadGame (struct boardType *board, enum gameStatusType *status,struct baseFileType *baseFile){
    int i,j;
    int fileNum;
    if (baseFile -> numFiles > 1){
        fileNum = input (6,0,1,2,*baseFile,1,baseFile -> numFiles,"Please Select a file:\n");
        fileNum--;
    }
    else
        fileNum = 0;
    FILE *file = fopen (baseFile -> file[fileNum].name,"r");
    fscanf (file,"%i\n%i\n%i\n%i\n%i\n%i\n%i\n",&(board -> width), &(board -> height), &(board -> numMine), &(board -> flagsUsed), &(board -> numCorrectFlag), &(board -> squaresLeft), status);
    for (i = 0; i < board -> width;i++)
        for (j = 0; j < board -> height;j++)
            fscanf (file,"%i\n%i\n\n",&(board -> square[i][j].mineCount) ,&(board -> square[i][j].status));
    fclose (file);
}

void saveGame (struct boardType *board, enum gameStatusType *status,struct baseFileType *baseFile){
    int i,j;
    int fileNum;
    if (baseFile -> numFiles > 0){
        fileNum = input (6,0,1,2,*baseFile,1,baseFile -> numFiles + (baseFile -> numFiles < MAX_FILES),"Please Select a file:\n");
        fileNum--;
    }
    else
        fileNum = 0;
    input (6,1,1,2,*baseFile,baseFile -> file[fileNum].name,"Please enter the name of your save file.\n");
    addtxt (baseFile -> file[fileNum].name);
    if (fileNum == baseFile -> numFiles)
        (baseFile -> numFiles)++;
    FILE *file = fopen (baseFile -> file[fileNum].name,"w");
    fprintf (file,"%i\n%i\n%i\n%i\n%i\n%i\n%i\n",board -> width, board -> height, board -> numMine, board->flagsUsed,board -> numCorrectFlag, board -> squaresLeft, *status);
    for (i = 0; i < board -> width;i++)
        for (j = 0; j < board -> height;j++)
            fprintf (file,"%i\n%i\n\n",board->square[i][j].mineCount,board->square[i][j].status);
    fclose (file);
}

void saveLoadGameOptions (struct boardType *board, enum gameStatusType *status, struct baseFileType *baseFile, enum boolean *exiting,int option){
    switch (option){
    case 1:
        saveGame (board,status,baseFile);
        break;
    case 2:
        if (baseFile -> numFiles > 0){
            loadGame (board,status,baseFile);
            *exiting = TRUE;
        }
        break;
    case 3:
        if (baseFile -> numFiles > 0)
            deleteGame (baseFile);
        break;
    case 4:
        if (baseFile -> numFiles > 0)
            renameSave (baseFile);
        break;
    case 5:
        *exiting = TRUE;
        break;
    }
}

///The main FIle IO function
void saveLoadGame (struct boardType *board, enum gameStatusType *status,struct baseFileType *baseFile, enum saveLoadGameType limit){
    int option;
    int fileNum;
    enum boolean exiting = FALSE;
    while (exiting == FALSE){
        option = input(7,0,0,0,*baseFile,1 + (limit == limited),SAVE_OPTIONS);
        saveLoadGameOptions (board,status,baseFile,&exiting,option);
    }
    baseSaveFileUpdate (*baseFile);
}

///--------------------------------------------------------///
///                       THE GAME                         ///
///--------------------------------------------------------///

///------CREATES THE INITIAL GAME------///

///CREATES THE INITIAL GAME CONDITIONS
void createGame (struct boardType *board){
    int i,j;

    board -> width = input (0,0,1,3,MIN_SIZE_X,MAX_SIZE_X,"How wide would you like the board? (min = %i, max = %i)\n",MIN_SIZE_X,MAX_SIZE_X);
    board -> height = input (0,0,1,3,MIN_SIZE_Y,MAX_SIZE_Y,"How high would you like the board? (min = %i, max = %i)\n",MIN_SIZE_Y,MAX_SIZE_Y);
    board -> numMine = input (0,0,1,3,MIN_MINES,maxMines (board -> width,board -> height),"How many mines would you like in board? (min = %i, max = %i)\n",MIN_MINES,maxMines (board -> width,board -> height));

    for (i = 0;i < board -> width; i++)
        for (j = 0;j < board -> height; j++){
            board -> square [i][j].status = nothing;
            board -> square[i][j].mineCount = 0;
        }

    board -> numCorrectFlag = 0;
    board -> flagsUsed = 0;
    board -> squaresLeft = board -> width * board -> height;
}

///RANDOMLY DISTRIBUTES MINES
void randomizeMines (struct boardType *board, enum gameStatusType *status,int x, int y){
    int i,j,k;
    int tmpX, tmpY;

    srand((unsigned)time(NULL));
    for (i = 0; i < (board -> numMine) ;i++){
        do{
            tmpX = rb (0,board -> width - 1);
            tmpY = rb (0,board -> height - 1);
        }while ((board -> square [tmpX][tmpY].mineCount == MINE) || valBet (tmpX,x - 1,x + 1) && valBet (tmpY,y - 1,y + 1));

            for (j = -1; j <= 1; j++)
                for (k = -1; k <= 1; k++)
                    if (valBet (tmpX + j,0,board -> width - 1) && valBet (tmpY + k,0,board -> height - 1) && board -> square [tmpX + j][tmpY + k].mineCount != MINE)
                        (board -> square [tmpX + j][tmpY + k].mineCount)++;
                         board -> square [tmpX][tmpY].mineCount = MINE;
                    }
    *status = minesCreated;
}

///CALCULATES THE MAXIMUM MINES ALLOWED DEPENDING ON THE BOARDSIZE
///Formula created from data collected from the windows version
int maxMines (int x, int y){
    return (x * y * 47 - 450) / 50;
}

///------------PauseMenu-----------///

void outputPauseMenu (){
    printf ("\n  X  X  X  X  X  X  X  X  X  X  X  !  1              1  2  X  1                \n");
    printf ("\n  X  X  X  X  X  X  X  X  X  X  2  1  1              1  !  2  1           1  1 \n");
    printf ("\n  X  X  X  X  X  X  X  X  X  X  1                    1  1  1              1  X \n");
    printf ("\n  X  X  X  X  X  X  X  X  X  X  ----------------                 1  1  1  1  X ");
    printf ("\n                               |   Restart: 1   |                              ");
    printf ("\n  X  X  X  X  X  !  1  1  2  2 |  Save Game: 2  |             1  2  X  X  X  X ");
    printf ("\n                               | Instruction: 3 |                              ");
    printf ("\n  X  X  X  1  1  1  1          |  Exit Game: 4  |             1  X  X  X  X  X ");
    printf ("\n                               | Resume Game: 5 |                              ");
    printf ("\n  X  1  1  1                   |                |    1  1  1  1  X  X  X  X  X ");
    printf ("\n                               |                |                              ");
    printf ("\n  X  1                 1  1  1 |                |    1  !  X  X  X  X  X  X  X ");
    printf ("\n                               |                |                              ");
    printf ("\n  !  1        1  1  1  1  X  2  ----------------     1  X  X  X  X  X  X  X  X \n");
    printf ("\n  1  1        1  X  1  1  2  X  1        1  X  2  2  2  X  X  X  X  X  X  X  X \n");
    printf ("\n              2  X  2       ------------------------    X  X  X  X  X  X  X  X ");
    printf ("\n                           |Please enter an option: |                          ");
    printf ("\n              1  X  1       ------------------------    X  X  X  X  X  X  X  X ");
    printf ("\n                                       ");
}

void pauseMenu (struct boardType *board, enum gameStatusType *status,struct baseFileType *baseFile, enum boolean *exiting){
    int option;
    enum boolean paused = TRUE;
    do{
        option = input (4,0,0,0,1,PAUSE_MENU_OPTIONS);
        switch (option){
        case 1:
            createGame (board);
            *status = play;
            paused = FALSE;
             break;
        case 2:
            saveLoadGame (board,status,baseFile,unlimited);
            break;
        case 3:
            instructions();
            break;
        case 4:
            *exiting = TRUE;
            paused = FALSE;
            break;
        case 5:
            paused = FALSE;
            break;
        }
    }while (paused);
}

///---------OUTPUTING THE GAME--------///

///converts the enum to the symbol
char symbol (struct squareType square, enum gameStatusType state){
    if (state == lose || state == win){
        if (square.mineCount == MINE && square.status == flag)
            return 'X';
        else if (square.mineCount == MINE)
            return '@';
        else if (square.status == flag)
            return '!';
        else
            return 32;
        }
    else{
        switch (square.status){
        case nothing:
            return 'X';
        case dug:
            if (square.mineCount > 0)
                return square.mineCount + '0';
                return 32;
        case question:
            return '?';
        case flag:
            return '!';
        }
    }
}

void centerBoardWidth (int boardWidth){
    repeatPrint (' ',(72 - 3 * boardWidth) / 2);
}

void centerBoardHeight (int boardHeight){
    repeatPrint ('\n',(boardHeight - 16) / (-2));
}

///Draws the board and the options
void outputBoard (struct boardType board, enum gameStatusType status){
    int i,j;

    if (DEBUG)
        debuger (board, status);

    centerBoardHeight (board.height);
    printf ("   ");
    centerBoardWidth (board.width);
    for (i = 0; i < board.width; i++)
            printf ("%3i",i + 1);
    printf ("\n    ");
    centerBoardWidth (board.width);
    for (i = 0; i < board.width; i++)
            printf ("---");
    printf ("\n");
    for (i = 0; i < board.height; i++){
        centerBoardWidth (board.width);
        printf (" %2i|",board.height - i);
        for (j = 0; j < board.width; j++)
            printf (" %c ",symbol(board.square [j][i],status));
        printf ("|%i\n",board.height - i);
    }
    printf ("    ");
    centerBoardWidth (board.width);
    for (i = 0; i < board.width; i++)
            printf ("---");
    printf ("\n   ");
    centerBoardWidth (board.width);
    for (i = 0; i < board.width; i++)
            printf ("%3i",i + 1);
    printf ("\n Flags Used: %i\t\t\t\t\t\t\t Flags Left: %i",board.flagsUsed,(board.numMine) - board.flagsUsed);
}

///------------GAME OPTIONS-----------///

///Recursive function
void digBox (struct boardType *board, int x, int y){
    int i,j;

    board -> square [x][y].status = dug;
    (board -> squaresLeft)--;

    for (i = -1; i <= 1; i++)
        for (j = -1; j <= 1; j++){
            if (valBet (x + i,0,board -> width - 1) && (valBet (y + j,0,board -> height - 1))){
                if (board -> square [x + i][y + j].status == dug){}
                else if (board -> square [x + i][y + j].mineCount > 0){
                    board -> square [x + i][y + j].status = dug;
                    (board -> squaresLeft)--;
                }
                else if (board -> square [x + i][y + j].mineCount == 0)
                    digBox (board,x + i, y + j);
            }
        }
}

///ALLOWS THE USER TO DIG
void dig (struct boardType *board, enum gameStatusType *status, enum boolean *exiting){
    char tmpS [MAX_LENGTH];
    int tmpX, tmpY;

    input (2,1,0,1,*board,*status,tmpS,"\n\n Please enter the coordinates where you would like to dig. ex: (5,12)\n ");

    if (!parseCoordinate(*board,tmpS,&tmpX,&tmpY))
        return;

    if (*status == play)
        randomizeMines (board,status,tmpX,tmpY);

    if (board -> square [tmpX][tmpY].status == flag)
        (board -> flagsUsed)--;

    if (board -> square [tmpX][tmpY].mineCount == MINE){
        *status = lose;
        *exiting = TRUE;
    }

    else if (board -> square[tmpX][tmpY].mineCount == 0)
        digBox (board,tmpX, tmpY);
    else{
        board -> square [tmpX][tmpY].status = dug;
        (board -> squaresLeft)--;
    }
}

///PLACES EITHER A FLAG OR A QUESTION MARK ON A COORDINATE
void place (struct boardType *board, enum gameStatusType status,enum squareStatusType thing){
    if (status == play)
        return;

    char tmpS [MAX_LENGTH];
    int tmpX, tmpY;

    input (2,1,0,1,*board,status,tmpS,"\n\n Please enter the coordinates a valid set of coordinates. ex: (5,12)\n ");

    if (!parseCoordinate(*board,tmpS,&tmpX,&tmpY))
        return;
    if (board -> square [tmpX][tmpY].status == dug)
        return;
    if (board -> square [tmpX][tmpY].status == flag)
        (board -> flagsUsed)--;
    if (thing == flag){
        if (board -> square [tmpX][tmpY].mineCount == MINE)
                (board -> numCorrectFlag)++;
        (board -> flagsUsed)++;
    }
    board -> square [tmpX][tmpY].status = thing;
}

///DETERMINES WHAT THE USER WANTS TO DO
void gameOptions (struct boardType *board, enum gameStatusType *status,struct baseFileType *baseFile, enum boolean *exiting, int option){
    switch (option){
    case 1:
        dig (board, status, exiting);
        break;
    case 2:
        if ((board -> flagsUsed) < (board -> numMine))
            place (board, *status,flag);
        break;
    case 3:
        place (board,*status,question);
        break;
    case 4:
        place (board,*status,nothing);
        break;
    case 5:
        pauseMenu (board,status,baseFile,exiting);
        break;
    }
}

int winCheck (struct boardType board){
    if (board.numCorrectFlag == (board.numMine))
        return 1;
    if (board.squaresLeft == (board.numMine) - (board.numCorrectFlag))
        return 1;
    return 0;
}

void outputGameOptions (enum gameStatusType status){
    repeatPrint (' ',4);
    printf ("Dig: 1");
    if (status == minesCreated)
        printf ("   Place Flag: 2    Place Question: 3    Remove: 4   ");
    else
        repeatPrint (' ',53);
    printf ("Pause Menu: 5\n");
    placeString(2,SCREEN_WIDTH / 2," Please enter the option that you would like to select: ");
}

///Executes the game
void playGame (struct boardType *board, enum gameStatusType *status, struct baseFileType *baseFile){
    enum boolean exiting = FALSE;
    int option;

    while (!exiting && (*status == play || *status == minesCreated)){
        option = input (3,0,0,0,*board,*status,1,GAME_OPTIONS);
        gameOptions (board,status,baseFile,&exiting, option);

        if (winCheck (*board)){
            *status = win;
            exiting = TRUE;
        }
    }
    if (*status == lose || *status == win){
        system ("cls");
        outputBoard(*board, *status);
        if (*status == lose){
                placeString (2,SCREEN_WIDTH / 2, "You lost :{\n");
                (baseFile -> gamesLost)++;
            }
        else{
            placeString (2,SCREEN_WIDTH / 2, "Congradulations on your win\n");
            placeString (2,SCREEN_WIDTH / 2, "Press any key to continue\n");
            (baseFile -> gamesWon)++;
        }
    }
    pause ();
}

///--------------------------------------------------------///
///                 INSTRUCTIONS FUNCTIONS                 ///
///--------------------------------------------------------///

void outputInstructionsPage (int page){
    int wait;

    if (page== 0){
        wait = 14;

        animateText("Creating a Game:\n",wait--);
        animateText("Specify the size of the board. For beginners 9 x 9 is recommended.\n",wait--);
        animateText("Specify the number of mines on the board. 10 is recommended for beginners.\n\n",wait--);
        animateText("First Move:\n",wait--);
        animateText("You have one option, digging.\n",wait--);
        animateText("Don't worry about this move; you cannot dig into a mine on the first turn\n\n",wait--);
        wait--;
        animateText("Symbols:\n",wait--);
        animateText("X: Not dug\n",wait--);
        animateText("!: Flag\n",wait--);
        animateText("?: Question\n",wait--);
        animateText("4: Number of mines around that square\n",wait--);
        animateText("@: active Mine\nX: disarmed mine\n\n",wait--);
        animateText("1: Next Page\t 2: Return to Menu\n",wait--);
    }
    else if (page == 1){
        int wait = 12;

        animateText("The Options:\nYou have 4 options when playing the game: Digging, Flag, Question and Remove.\n",wait--);
        animateText("1: Digging allows you to reveal empty squares around the desired coordinate.\n",wait--);
        animateText("2: Flag a square when you are sure that there is a mine on that square.\n",wait--);
        animateText("3: Question will place a question mark on a square.\n\t",wait--);
        animateText("Use when unsure whether or not there is a mine.\n",wait--);
        animateText("4: Remove removes the flags or questions placed on a square.\n",wait--);
        animateText("Each of these options will ask you for the coordinate of a square.\n\n",wait--);
        wait--;
        animateText("Enter Coordinates:\nThe coordinate (0,0) is in the bottom left corner.\n",wait--);
        animateText("When asked for coordinates enter it in this format: (x,y)\n\n",wait--);
        animateText("Press any key to return to Menu\n",wait--);
        animateText("1: Previous Page\t 2: Return to Menu\n",wait--);
    }
}


///Prints out the instructions, may also include diagrams
void instructions (){
    int page = 0;
    int option;

    do{
        option = input (5,0,0,0,page,1,INSTRUCTION_OPTIONS);
        if (page == 0)
            page = 1;
        else
            page = 0;
    }while (option != 2);
}

///--------------------------------------------------------///
///                     MENU FUNCTIONS                     ///
///--------------------------------------------------------///

///Runs the needed function
void menuOptions (struct boardType *board, enum gameStatusType *status, struct baseFileType *baseFile, enum boolean *quiting, int option){
    switch (option){
    case 1:
        createGame(board);
        *status = play;
        playGame (board, status, baseFile);
        break;
    case 2:
        saveLoadGame (board,status,baseFile, limited);
        playGame (board, status, baseFile);
        break;
    case 3:
        instructions();
        break;
    case 4:
        *quiting = TRUE;
        break;
    }
}

///Outputs the main Menu of the Program
void outputMenu (enum gameStatusType status, struct baseFileType baseFile){
    int tmp;
    repeatPrint ('\n',3);
    placeString (2,SCREEN_WIDTH/2,"MINESWEEPER");
    repeatPrint ('\n',5);
    tmp = placeString (2,SCREEN_WIDTH/3,"New Game: 1");
    placeString (2,SCREEN_WIDTH/3 - tmp / 2,"Load Game: 2");
    repeatPrint ('\n',2);
    placeString (2,SCREEN_WIDTH/2,"Statistics:");
    repeatPrint ('\n',2);
    placeString (2,SCREEN_WIDTH/2,"Won: %i     Lost: %i",baseFile.gamesWon,baseFile.gamesLost);
    repeatPrint ('\n',2);
    placeString (2,SCREEN_WIDTH/2,"Played: %i",baseFile.gamesWon + baseFile.gamesLost);
    repeatPrint ('\n',2);
    tmp = placeString (2,SCREEN_WIDTH/3,"Instructions: 3");
    placeString (2,SCREEN_WIDTH/3 - tmp / 2,"Quit: 4");
    repeatPrint ('\n',4);
    placeString (2,SCREEN_WIDTH/2,"Please select an Option:");
    printf ("\n");
    repeatPrint (' ',SCREEN_WIDTH/2);
}

///Runs the main menu of the program
void menu (struct baseFileType *baseFile){
    struct boardType board;
    enum gameStatusType status = notPlaying;
    enum boolean quiting = FALSE;
    int option;

    while (!quiting){
        option = input (1,0,0,0,status,*baseFile,1,MENU_OPTIONS);
        menuOptions (&board, &status, baseFile, &quiting, option);
    }

}

///--------------------------------------------------------///
///                  Start/End Animations                  ///
///--------------------------------------------------------///

///Start Screen Animation
void startScreen (){
    int i;
    for (i = 0; i < MENU_DELAY; i++){
        repeatPrint ('\n',3);
        placeString(2,SCREEN_WIDTH/2,"MINESWEEPER");
        repeatPrint ('\n',9);
        placeString(2,SCREEN_WIDTH/2,"%i", MENU_DELAY - i);
        repeatPrint ('\n',10);
        placeString (2,SCREEN_WIDTH / 2,"DESIGNED AND CODED BY");
        repeatPrint ('\n',2);
        placeString (2,SCREEN_WIDTH / 2,"DENIS SHLEIFMAN AND NUBAIL YAKOOB");
        delay (1000);
        system ("cls");
    }
}

void credits (int position){
    int y = 0;
    while (y < position && y < SCREEN_HEIGHT){
        printf ("\n");
        y++;
    }
    if (valBet (y,0,SCREEN_HEIGHT - 1) && position >= 0){
        placeString (3,SCREEN_WIDTH / 2 - 2,"PROGRAMMERS:");
        printf (" DENIS SHLEIFMAN");
        y++;
    }
    if (valBet (y,0,SCREEN_HEIGHT - 1) && position >= -1){
        printf ("\n");
        placeString (1,SCREEN_WIDTH / 2 - 1,"NUBAIL YAKOOB");
        y++;
    }
    while (y < position + 10 && y < SCREEN_HEIGHT){
        printf ("\n");
        y++;
    }
    if (valBet (y,0,SCREEN_HEIGHT - 1) && position >= -10){
        printf ("\n");
        placeString (2,SCREEN_WIDTH / 2,"Thank you for playing");
        y++;
    }
}

///Need an End Screen
void endScreen (){
    int i;
    for (i = 30; i > -30; i--){
        system("cls");
        credits (i);
        delay(25);
    }
}

///---------------------------//////////////////-----------------------------///
///                           //////////////////                             ///
///                           ///////MAIN///////                             ///
///                           //////////////////                             ///
///---------------------------//////////////////-----------------------------///

int main (){
    struct baseFileType baseFile;
    startScreen ();
    baseSaveFileRead (&baseFile);
    menu (&baseFile);
    baseSaveFileUpdate (baseFile);
    endScreen ();
}
