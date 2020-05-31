#include <stdio.h>
#include "Linkedlist.c"

/**
	This is header file, to mention all new user-defined functions 
*/

/*************************************************/

//Not much action here, simply the initial random implementation in a new function
//Move initRandom(char myColor);
Move initRandom(char myColor, Position *aPosition);


Move* newRandom(char myColor, Position *aPosition);

LinkedList* moveFinder(Position *gamePosition);

void multipleJumps(LinkedList* moveList, Move* move, short k ,short i, short j, Position *gamePos);


int evaluationFunction (Position *aPosition);

int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove, int isRoot);


short quiescenceSearch(Position* gamePos);

//int alpha_beta1(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove);


int minimax(Position *gamePos, int depth, int ismaximizingPlayer, Move* finalMove, int isRoot);

void simpleMove(LinkedList* moveList, Position *gamePos, short i, short j, short playerDirection, short moveDirection);

void moveLegality(LinkedList* moveList, Move* move, Position* gamePos);

int iterativeDeepeningSearch(Position* aPosition, Move* finalMove);

int MTDFSearch(Position* gamePos, int f, int d, Move* finalMove);

int NegaScout(Position *gamePos, char depth, int alpha, int beta, int isRoot, Move* finalMove);


int max(int num1, int num2);

int min(int num1, int num2);


/************************************************
	Everything related to linked list structures used.
	Simple Implementation of a typical linked list.
************************************************/
//Initializes everything in LinkedList
LinkedList* LinkedListInitializer(LinkedList *myList);

//Prints the whole list
void printList(LinkedList *myList);


void printAvailableMoves(LinkedList *myList);

//Deletes the whole list
void deleteList(LinkedList *myList);

//Adds element at the start of the linked list
void addElement(LinkedList *myList ,Move *data);

//Removes first element from list
Move* removeFirst(LinkedList *myList );

//void emptyList(LinkedList *myList);



short moveIterator(LinkedList* moveList,Position* gamePos, Move* move ,short i, short j, short playerDirection, short jumpPossible);

struct
{
	unsigned int jumpDirection : 3;
	int playDirection : 3;
	unsigned int jumpFlag :1;
	int playMoveDirection : 3;
} MoveUtil;

