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

void multipleJumps(LinkedList* moveList, Move* move, int k, int i, int j,Position *gamePosition);

//LinkedList* simpleMove(LinkedList *mylinkedlist,Position *gamePosition, int i, int j);

int evaluationFunction (Position *aPosition);

int alpha_beta(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove, int isRoot);


int quiescenceSearch(Position* gamePos);

//int alpha_beta1(Position *aPosition, char depth, int alpha, int beta, char maximizingPlayer, Move* finalMove);


int minimax(Position *gamePos, int depth, int ismaximizingPlayer, Move* finalMove, int isRoot);

void simpleMove(LinkedList* moveList, Position *gamePos, int i, int j, int playerDirection,int moveDirection);

void moveLegality(LinkedList* moveList, Move* move, Position* gamePos);

int iterativeDeepeningSearch(Position* aPosition, Move* finalMove);

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