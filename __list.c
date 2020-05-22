#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void initList(list *sList){
	sList->head= NULL;
	sList->next= NULL;
	sList->data = NULL;
}

void push(list *sList, Move* data)
{
	list *p = (list*)malloc(sizeof(list));
	p->data = data;
	p->next = sList->head;
	sList->head = p;
}


//DONE
Move* pop(list *sList)
{
	if(sList == NULL || sList->head == NULL){
		//printf("Empty List.\n");
		return NULL;
	}
	// Move *myData;
	
	// list *p = sList->head;
	// sList->head = sList->head->next;
	// // if(sList->start == NULL)
	// // 	sList->tail = NULL;
	// myData = p->data;
	// free(p);


	Move* myData = malloc(sizeof(Move));

	list *temp = sList->head;

	memmove(myData,temp->data,sizeof(Move));

	sList->head = sList->head->next;

	free(temp);
	
	return myData;
}

//DONE
void freeList(list * sList){
	while(sList->head != NULL)
		free(pop(sList));
	free(sList);
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
	list* aNode = sList->head;
	while(aNode != NULL){
		printMove(aNode->data);
		aNode = aNode->next;
	}
}




//TODO REMOVE
Move* top(list *sList){
	//assert(sList->start != NULL);
	if(sList == NULL || sList->head == NULL)
		return NULL;
	return sList->head->data;
}