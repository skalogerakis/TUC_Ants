#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LinkedList LinkedList;

struct LinkedList
{
	Move* data;
	struct LinkedList *next;
	struct LinkedList *head;
};


LinkedList* LinkedListInitializer(LinkedList *myList){
	myList->data = NULL;
	myList->head = NULL;
	myList->next = NULL;
	return myList;
}

//LEGACY: This will not work to print Move struct
// void printList(LinkedList *myList){
//    LinkedList *ptr = myList->head;

//    if(ptr == NULL){
//    	printf("List is empty\n");
//    	return;
//    }

//    //Print everything starting from the start
//    while(ptr != NULL){
// 		printf("Element (%d)\n", ptr->data->tile[0][0]);
//    		ptr = ptr->next;
//    }
   
// }


void printAvailableMoves(LinkedList *myList){
   LinkedList *ptr = myList->head;

   if(ptr == NULL || myList == NULL){
   	printf("List is empty\n");
   	return;
   }

   printf("START PRINTING AGAIN!!\n");

   //Print everything starting from the start
   while(ptr != NULL){
   		int i=0;
		while(ptr->data->tile[0][i] != -1){
			if(i==0){
				printf("\nInit Element pos (%d,%d)\n", ptr->data->tile[0][i], ptr->data->tile[1][i]);
			}else{
				printf("\tNext Element pos (%d,%d)\n", ptr->data->tile[0][1], ptr->data->tile[1][1]);

			}
			
			i++;
		}

   		ptr = ptr->next;
   }
   
}


//Another implementaion. Did not help much
  /* Function to delete the entire linked list */
//https://www.geeksforgeeks.org/write-a-function-to-delete-a-linked-list/
void deleteList(LinkedList *myList)  
{  
      
/* deref head_ref to get the real head */
LinkedList* current = myList->head;  
LinkedList* next;  
  
while (current != NULL)  
{  
    next = current->next;  
    free(current);  
    current = next;  
}  
      
/* deref head_ref to affect the real head back  
    in the caller. */
myList->head = NULL;  
}  


void addElement(LinkedList *myList ,Move* data){
	LinkedList *newElem = (LinkedList*)malloc(sizeof(LinkedList));
	newElem->data = data;
	newElem->next = myList->head;
	myList->head = newElem; 
}

Move* removeFirst(LinkedList *myList ){

	if(myList == NULL || myList->head == NULL){
		//printf("List empty\n");
		return NULL;
	}

	Move* returnMove = malloc(sizeof(Move));

	LinkedList *temp = myList->head;
	memmove(returnMove,temp->data,sizeof(Move));
	myList->head = myList->head->next;

	return returnMove;
}
