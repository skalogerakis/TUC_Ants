#ifndef _LIST_H
#define _LIST_H
#include "move.h"
/* ^^ these are the include guards */

/* Prototypes for the functions */


typedef struct node{
	Move* data;
	struct node *next;
	struct node *head;
} list;




void initList(list *sList);
void push(list *sList, Move* data);
Move* pop(list *sList);
void freeList(list*);
list* merge(list*, list*);
Move* top(list *sList);
void printList(list*);
void printMove(Move *aMove);
void emptyList(list*);
#endif