#include <stdio.h>
#include <stdlib.h>
//TODO data is int type. Change after test that works fine

typedef struct LinkedList LinkedList;

struct LinkedList
{
	int data;
	struct LinkedList *next;
	struct LinkedList *head;
};


void LinkedListInitializer(LinkedList *myList){
	myList->head = NULL;
	myList->next = NULL;
	myList->data=-1;
}

void printList(LinkedList *myList){
   LinkedList *ptr = myList->head;

   if(ptr == NULL){
   	printf("List is empty\n");
   	return;
   }

   //Print everything starting from the start
   while(ptr != NULL){
		printf("Element (%d)\n", ptr->data);
   		ptr = ptr->next;
   }
   
}

void deleteList(LinkedList *myList){
	LinkedList *ptrNext;

   	if(myList->head == NULL){
   		printf("List is empty\n");
   		return;
   	}

   	while(myList->head != NULL){
		ptrNext = myList->head->next;
		free(myList->head);
   		myList->head = ptrNext;
   	}
   	free(myList);

   	printf("List deleted successfully\n");
}

void addElement(LinkedList *myList ,int data){
	LinkedList *newElem = (LinkedList*)malloc(sizeof(LinkedList));

	newElem->data = data;

	// if(myList == NULL){

	// }

	newElem->next = myList->head;
	myList->head = newElem; 
}

void removeFirst(LinkedList *myList ){
	printf("Remove First Element\n");
	LinkedList *initNode = myList->head;

	if(initNode == NULL){
		printf("List empty\n");
		return;
	}

	myList->head = myList->head->next;
	free(initNode);
}