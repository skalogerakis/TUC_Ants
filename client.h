#include <stdio.h>
#include "Linkedlist.c"

/**
	This is header file, to mention all new user-defined functions 
*/

/*************************************************
			Search Functions
*************************************************/
int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove, int isRoot);

int quiescenceSearch(Position* gamePos);

int minimax(Position *gamePos, int depth, int ismaximizingPlayer, Move* finalMove, int isRoot);

int iterativeDeepeningSearch(Position* aPosition, Move* finalMove);

int MTDFSearch(Position* gamePos, int f, int d, Move* finalMove);

int NegaScout(Position *gamePos, char depth, int alpha, int beta, int isRoot, Move* finalMove);

//Not much action here, simply the initial random implementation in a new function
Move initRandom(char myColor, Position *aPosition);


/*************************************************
	Utility Functions for implementation
*************************************************/

LinkedList* moveFinder(Position *gamePosition);

void multipleJumps(LinkedList* moveList, Move* move, short k ,short i, short j, Position *gamePos);

int evaluationFunction (Position *aPosition);

int evaluationCheck(Position *gamePos,short i, short j);


void simpleMove(LinkedList* moveList, Position *gamePos, short i, short j, short playerDirection, short moveDirection);

void moveLegality(LinkedList* moveList, Move* move, Position* gamePos);

int max(int num1, int num2);

int min(int num1, int num2);

// Some bit-precision variables. Did not help much so didnt change all variables
struct
{
	unsigned int jumpDirection : 3;
	int playDirection : 3;
	unsigned int jumpFlag :1;
	int playMoveDirection : 3;
} MoveUtil;

//A heuristic representation of the board. Moves in sides worth more as our piece cannot be threatened
short tableHeuristics[12][8]={
	{150, 150, 150 ,150 ,150, 150, 150, 150},
	{70, 60, 60 ,60 ,60, 60, 60, 70},
	{50, 40, 30 ,30 ,30, 30, 40, 50},
	{50, 40, 30 ,20 ,20, 30, 40, 50},
	{50, 40, 30 ,20 ,20, 30, 40, 50},
	{50, 40, 30 ,20 ,20, 30, 40, 50},
	{50, 40, 30 ,20 ,20, 30, 40, 50},
	{50, 40, 30 ,20 ,20, 30, 40, 50},
	{50, 40, 30 ,50 ,50, 30, 40, 50},
	{50, 40, 30 ,30 ,30, 30, 40, 50},
	{70, 60, 60 ,60 ,60, 60, 60, 70},
	{150, 150, 150 ,150 ,150, 150, 150, 150},
};


/************************************************
	Everything related to linked list structures used.
	Simple Implementation of a typical linked list.
************************************************/
//Initializes everything in LinkedList
LinkedList* LinkedListInitializer(LinkedList *myList);

//Prints the whole list
void printList(LinkedList *myList);

//Print all available moves in a list
void printAvailableMoves(LinkedList *myList);

//Deletes the whole list
void deleteList(LinkedList *myList);

//Adds element at the start of the linked list
void addElement(LinkedList *myList ,Move *data);

//Removes first element from list
Move* removeFirst(LinkedList *myList );


