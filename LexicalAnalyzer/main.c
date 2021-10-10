#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#define ADDOP 11
#define MULOP 12
#define RELOP 13
#define NULL 14
#define PROG 15
#define VAR 16
#define ARRAY 17
#define OF 18
#define INTEGER 19
#define REAL 20
#define FUNCTION 21
#define PROC 22
#define BEGIN 23
#define END 24
#define IF 25
#define WHILE 26
#define ELSE 27
#define DO 28
#define NOT 29
#define PLUS 30
#define MINUS 31
#define OR 32
#define STAR 33
#define SLASH 34
#define DIV 35
#define MOD 36
#define AND 37
#define GT 38
#define GE 39
#define LT 40
#define LE 41
#define EQ 42
#define NE 43
#define CB 44
#define OB 45
#define COMMA 46
#define DOT 47
#define DOTDOT 48
#define COLON 49
#define SCOLON 50
#define ASSIGNOP 51
#define OP 52
#define CP 53
#define INTNUM 54
#define REALNUM 55
#define LONGNUM 56
#define THEN 57
#define EOF 58
#define CALL 59
#define ID 60
#define NUM 61
//replacement null variable
#define NILL (void * )0

/* Order of construction:
 * defines
 * text file for reserved words
 * load file to array
 * linked list for identifiers
 * read line / buffer (read in program)
 * build machines :
 *  whitespace DONE
 *  IDRes DONE
 *  longreal
 *  real
 *  integer
 *  catchall
 *  relop DONE
 * */


struct res_word {
    char lexeme[10];
    int token;
    int attribute;
    struct res_word *search;
};
//21 total reserved words
//struct res_word words[20];
struct res_word *headRW = NILL;

//struct LL for the identifiers
struct node {
    char lex[10];
    int tok;
    //for the next struct in the linked list
    struct node *next;
};
//nodes for the head and current struct in the linked list
struct node *head = NILL;
struct node *current = NILL;


void PrintStruct();
//read from file and put into an array. the file is ReservedWords.txt
void ReadWordFile() {
    FILE *in_file;
    in_file = fopen("C://Users//cdmel//CLionProjects//LexicalAnalyzer//ReservedWords.txt","r");
    if (in_file == NILL)
    {
        printf("Error! Could not open file\n");
        exit(-1);
    }

    //read one line each from the file, create struct object for that reserved word, save lexeme,token,att to each
    char buffer[72];
    int temptoken;
    int tempattribute;
    int i = 0;

    //72 character buffer for size (void * )0) is the definition of NILL (null)
    while(fgets(buffer, sizeof(buffer), in_file) != NILL){
        char templexeme[10];
        temptoken = 0;
        tempattribute = 0;
        //create a word to put into words array
        //struct res_word word;
        struct res_word *resWordAdd = (struct res_word*) malloc(sizeof(struct res_word));

        //printf("getting inside loop\n");
        //printf("buffer: %s", buffer);

        sscanf(buffer,"%s %d %d", templexeme, &temptoken, &tempattribute);
        strcpy(resWordAdd->lexeme, templexeme);
        resWordAdd->token = temptoken;
        resWordAdd->attribute = tempattribute;

        resWordAdd->search = headRW;
        headRW = resWordAdd;
        //words[i] = resWordAdd;
        //i++;
    }
    fclose(in_file);
    //PrintStruct(words, 20);
}

//adding to the linked list of structs for SymbolTable to hold Identifiers
void AddToLinkedList(char identifier[10], int tokentype) {
    struct node *idword = (struct node*) malloc(sizeof(struct node));
    //copying string identifier to lex, tokentype to tok
    strcpy(idword->lex, identifier);
    idword->tok = tokentype;
    //setting next to idword as head, making idword the head
    idword->next = head;
    head = idword;
}

//if the identifier is in the table, it needs to return it's lex, tok, pointer to sym table
//check if identifier is in the SymbolTable
struct node* findLL(char identifier[10]) {
    struct node *current = head;

    //if list is empty
    if(head == NILL){
        return NILL;
    }

    //looking through the list
    while(strcmp(current->lex, identifier) != 0){
        int* pointer = (int*)&current;
        int i = *pointer;
        printf("current in loop: Lex=%s, Tok=%d, Att=%d\n", current->lex, current->tok, i);
        current = current->next;
        if(current == NILL) {
            return NILL;
        }
    }
    return current;
}

struct res_word *currentRW = NILL;

struct res_word* findResWord (char lexsearch[10]) {
    currentRW = headRW;
    //printf("currentRW out loop: Lex=%s, Tok=%d, Att=%d\n", currentRW->lexeme, currentRW->token, currentRW->attribute);

    while(strcmp(currentRW->lexeme, lexsearch) != 0){
        printf("currentRW in loop: Lex=%s, Tok=%d, Att=%d\n", currentRW->lexeme, currentRW->token, currentRW->attribute);
        currentRW = currentRW->search;
        if(currentRW == NILL) {
            return NILL;
        }
    }
    return currentRW;
}

//printing the symbol table
void PrintSymbolTable() {
    struct node *ptr = head;
    printf("\n");

    //attrbiute will return a pointer to that object in the linked list
    while(ptr != NILL) {
        int* pointer = (int*)&ptr;
        int i = *pointer;
        printf("Lexeme: %s, ID: %d, Attribute: %d\n", ptr->lex, ptr->tok, i);
        printf("\n");
        ptr = ptr->next;
    }
}

//printing the contents of the reserved word structure
void PrintStruct() {
    struct res_word *ptr = headRW;
    printf("\n");

    while(ptr != NILL) {
        printf("Lexeme: %s, ID: %d, Attribute: %d", ptr->lexeme, ptr->token, ptr->attribute);
        printf("\n");
        ptr = ptr->search;
    }
}

//pointers for moving through the buffer during machines
int forwardPtr = 0;
int backPtr = 0;
int linenum;
void AddToTokenFile(int linenumber,struct res_word tokenStruct) {
    printf("line No. %d Lexeme: %s Token-Type: %d attribute: %d\n", linenumber,
           tokenStruct.lexeme, tokenStruct.token, tokenStruct.attribute);
}

//when something in the machines fails, call this and decide what state to go to next
//send in what state the machines failed at to mark which one needs to come next
int fail(int statefailed) {
    int start = statefailed;
    forwardPtr = backPtr;
    //forward pointer goes back to the back pointer

    switch(start) {
        case 0: start = 9; break;
        case 9: start = 12; break;
        case 12: start = 20; break;
        case 20: start = 25; break;
    }
    return start;
}

//I know this is really dumb but isaplha and isalumn
// would not work for some reason so this is my stupid fix.
int isLetter(unsigned char check) {
    check = tolower(check);
    if(check == 'a' || check =='b' || check == 'c' || check == 'd'
    || check == 'e' || check == 'f' || check == 'g' || check == 'h' ||
    check == 'i' || check == 'j' || check == 'k' || check == 'l' ||
    check == 'm' || check == 'n' || check == 'o' || check == 'p' ||
    check == 'q' || check == 'r' || check == 's' || check == 't' ||
    check == 'u' || check == 'v' || check == 'w' || check == 'x' ||
    check == 'y' || check == 'z') {
        return 1;
    }
    else {
        return 0;
    }
}

struct res_word getToken(char buffer[72]) {
    struct res_word *tokenStruct;
    tokenStruct = (struct res_word*) malloc(sizeof(struct res_word));
    char c;
    int state = 0;
    char string[10];
    unsigned int length = 0;

    c = buffer[forwardPtr];
    printf("forwardPointer= %d, backPointer= %d", forwardPtr, backPtr);
    printf("\n");

    //change this part to a function, call the function and return the value to a struct add to token file
while(1) {
    //start running through machines, put some kind of loop here to keep checking until return a token
    switch (state) {
        case 0:
            //c starts at the front of the buffer, front is incrememented
            c = buffer[forwardPtr];
            printf("c in state %d: %c\n", state, c);
            //c is currently at the start of the buffer, check if whitespace, if yes incremement and set state back to 0
            if (c == ' ' || c == '\t' || c == '\n') {
                backPtr = forwardPtr;
                forwardPtr++;
                state = 0;
            }
            else if (isLetter(c) == 1) state = 9;
            else if (c == '<') state = 1;
            else if (c == '=') state = 5;
            else if (c == '>') state = 6;
                //if its not any of the above, go to fail and that will start state numbers
            //else state = fail(0);
            break;
        case 1:
            printf("c in state %d: %c\n", state, c);
            //c == '<' less than, check if next is = or > or other(if other go back and fix- set forward pointer to back pointer
            // check what it is and go to that state, return tok,id)
            printf("Before character: %c\n", c);
            forwardPtr++;
            c = buffer[forwardPtr];
            printf("New character: %c\n", c);
            if (c == '=') {
                //output tok, id (RELOP, LE)
                backPtr = forwardPtr;
                strcpy(tokenStruct->lexeme, "<=");
                tokenStruct->token = RELOP;
                tokenStruct->attribute = LE;
                forwardPtr++;
                return *tokenStruct;
            }
            else if (c == '>') {
                //output tok, id (RELOP, NE)
                backPtr = forwardPtr;
                strcpy(tokenStruct->lexeme, "<>");
                tokenStruct->token = RELOP;
                tokenStruct->attribute = NE;
                forwardPtr++;
                return *tokenStruct;
            }
                //if c is anything else, go back
            else {
                //output tok, id (RELOP,, LT)
                backPtr = forwardPtr;
                strcpy(tokenStruct->lexeme, "<");
                tokenStruct->token = RELOP;
                tokenStruct->attribute = LT;
                return *tokenStruct;
            }
        case 5:
            printf("c in state %d: %c\n", state, c);
            //c == '=' equals, output (RELOP, EQ)
            backPtr = forwardPtr;
            strcpy(tokenStruct->lexeme, "=");
            tokenStruct->token = RELOP;
            tokenStruct->attribute = EQ;
            forwardPtr++;
            return *tokenStruct;
        case 6:
            printf("c in state %d: %c\n", state, c);
            //c == '>' check if = and then other
            forwardPtr++;
            c = buffer[forwardPtr];
            if (c == '=') {
                backPtr = forwardPtr;
                strcpy(tokenStruct->lexeme, ">=");
                tokenStruct->token = RELOP;
                tokenStruct->attribute = GE;
                forwardPtr++;
                return *tokenStruct;
            }
                //else c is something else, go back and send to token file GT
            else {
                backPtr = forwardPtr;
                strcpy(tokenStruct->lexeme, ">");
                tokenStruct->token = RELOP;
                tokenStruct->attribute = GT;
                printf("forwardPointer in State 6 GT= %d, backPointer= %d\n", forwardPtr, backPtr);
                return *tokenStruct;
            }
        case 9:
            //ID-RES MACHINE
            printf("forwardPointer= %d, backPointer= %d\n", forwardPtr, backPtr);
            printf("Before character in state %d: %c\n",state,  c);

            c = buffer[forwardPtr];
            printf("Is Letter? Value = %d\n",isLetter(c));

            if(isLetter(c)==1 || isdigit(c)) {
                //incrementing length every time we encounter a letter to check size of string before adding to string
                length++;
                printf("length of id so far: %d\n", length);
                if(length > 10) {
                    //LEXERR identifier too long RETURN
                    //FIXME: add in the LEXERROR DEFINES and return correct lexerror when string is too long
                }
                printf("getting in the loop\n");
                printf("char= %c\n", c);

                strncat(string, &c, 1);
                printf("In loop, string= %s\n", string);

                backPtr = forwardPtr;
                forwardPtr++;
                state = 9;
            }

            else{
                backPtr = forwardPtr;
                printf("Checking reserved word table for: %s\n",string);
                struct res_word *find = NILL;  //use find for the reserved words and findString for Identifiers
                struct node *findString = NILL;
                find = findResWord(string);
                findString = findLL(string);
                currentRW = NILL;

                //checking reserved word table
                if(find != NILL) {
                    printf("Found in reserved word table: %s\n", string);

                    strcpy(tokenStruct->lexeme, find->lexeme);
                    tokenStruct->token = find->token;
                    tokenStruct->attribute = find->attribute;
                    return *tokenStruct;
                }
                //checking symbol table
                else if(findString != NILL){
                    printf("Checking symbol table for: %s\n",string);
                    printf("Found in Identifiers Symbol Table: %s\n", string);
                    strcpy(tokenStruct->lexeme, findString->lex);
                    tokenStruct->token = findString->tok;

                    int* pointer = (int*)&findString;
                    int i = *pointer;

                    tokenStruct->attribute = i;
                    return *tokenStruct;
                }
                //add word to symbol table
                else{
                    printf("Identifier not in Symbol Table: %s\n", string);
                    printf("Adding Identifier %s to symbol table\n",string);
                    AddToLinkedList(string, ID);
                    findString = findLL(string);

                    strcpy(tokenStruct->lexeme, string);
                    tokenStruct->token = ID;
                    int* pointer = (int*)&findString;
                    int i = *pointer;

                    tokenStruct->attribute = i;
                    return *tokenStruct;
                }
            }
            break;
        }
    }
    }


    int main() {
        //reading in the reserved word file
        ReadWordFile();
        PrintStruct();
        linenum=1;

        //FIXME: add in the option to choose which program to load
        //read in the program file and get 72 char in a buffer,
        //run through machines
        FILE *in_file;
        //in_file = fopen("C://Users//cdmel//CLionProjects//LexicalAnalyzer//Program.txt","r");

        in_file = fopen("C://Users//cdmel//CLionProjects//LexicalAnalyzer//ProgramTest.txt", "r");
        if (in_file == NILL) {
            printf("Error! Could not open file\n");
            exit(-1);
        }

        char buffer[72];
        while (fgets(buffer, sizeof(buffer), in_file) != NILL) {
            //state will start at 0 before going into the machines
            forwardPtr = 0;
            backPtr = 0;

            printf("\nBuffer contents: %s\n", buffer);
            char readBuff = buffer[forwardPtr];
            struct res_word token;

            unsigned int length = strcspn(buffer, "\n");
            printf("length of buffer= %d \n", length);

            while(readBuff != '\n') {
                printf("BEFORE forwardPointer= %d, backPointer= %d\n", forwardPtr, backPtr);
                readBuff = buffer[forwardPtr];
                token = getToken(buffer);
                AddToTokenFile(linenum, token);
                if(forwardPtr == length){
                    break;
                }
            }
            //increase line number each time a line is read completely.
            linenum++;
            //getting a char from the buffer taken from the program file
            //send through machines to figure out what the char is
            //check for errors as you go, change token to error information to be sent to listing file
        }

        fclose(in_file);
        //linked list testing
        AddToLinkedList("testing",ID);
        AddToLinkedList("test2",ID);
        AddToLinkedList("test3",ID);

        PrintSymbolTable();
    }