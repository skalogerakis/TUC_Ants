#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void initList(list *sList){
	sList->start= NULL;
	sList->tail= NULL;
}

void push(list *sList, Move* data)
{
	node *p;
	p = malloc(sizeof(node));
	p->data = data;
	p->next = sList->start;
	if(sList->start == NULL);
		sList->tail = p;
	sList->start = p;
}

Move* top(list *sList){
	//assert(sList->start != NULL);
	if(sList == NULL || sList->start == NULL)
		return NULL;
	return sList->start->data;
}

Move* pop(list *sList)
{
	if(sList == NULL || sList->start == NULL)
		return NULL;
	Move *myData;
	
	node *p = sList->start;
	sList->start = sList->start->next;
	if(sList->start == NULL)
		sList->tail = NULL;
	myData = p->data;
	free(p);
	
	return myData;
}

list * merge(list* A, list* B){
	assert (!(A==NULL && B==NULL));
	if (A == NULL || A->start == NULL)
		return B;
	if (B == NULL || B->start == NULL)																																																																																																																																																													
		return A;
	A->tail->next = B->start;
	A->tail = B->tail;
	return A;
}

void freeList(list * sList){
	while(sList->start != NULL)
		free(pop(sList));
	free(sList);
}
void emptyList(list * sList){
	while(sList->start != NULL)
		free(pop(sList));
}

void printMove(Move *aMove){
	int i;
	for (i = 0; i < MAXIMUM_MOVE_SIZE; i++){
		if(aMove->tile[0][i] == -1)
			break;
		printf("%d %d\n", aMove->tile[0][i], aMove->tile[1][i]);
	}
	printf("--\n");
}
void printList(list * sList){
	node* aNode = sList->start;
	while(aNode != NULL){
		printMove(aNode->data);
		aNode = aNode->next;
	}
}