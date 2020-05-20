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

void multipleJumps(LinkedList* moveList, Move* move, Position *gamePosition, int i, int j, int k );

LinkedList* simpleMove(LinkedList *mylinkedlist,Position *gamePosition, int i, int j);




/************************************************
	Everything related to linked list structures used.
	Simple Implementation of a typical linked list.
************************************************/
//Initializes everything in LinkedList
void LinkedListInitializer(LinkedList *myList);

//Prints the whole list
void printList(LinkedList *myList);


void printAvailableMoves(LinkedList *myList);

//Deletes the whole list
void deleteList(LinkedList *myList);

//Adds element at the start of the linked list
void addElement(LinkedList *myList ,Move *data);

//Removes first element from list
void removeFirst(LinkedList *myList );

// void emptyList(LinkedList *myList);