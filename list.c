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

	Move* myData = malloc(sizeof(Move));

	list *temp = sList->head;

	memmove(myData,temp->data,sizeof(Move));

	sList->head = sList->head->next;

	free(temp);
	
	return myData;
}

//DONE
// void freeList(list * sList){
// 	while(sList->head != NULL)
// 		free(pop(sList));
// 	free(sList);
// }

void freeList(list *myList)  
{  
      
/* deref head_ref to get the real head */
list* current = myList->head;  
list* next;  
  
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

// void emptyList(list * sList){
// 	while(sList->head != NULL)
// 		free(pop(sList));
// }

void printMove(list* myList){
	list *ptr = myList->head;

   if(ptr == NULL || myList == NULL){
   	//printf("List is empty\n");
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

//LEGACY: This will not work to print Move struct
void printList(list *myList){
   list *ptr = myList->head;

   if(ptr == NULL){
   	//printf("List is empty\n");
   	return;
   }

   //Print everything starting from the start
   while(ptr != NULL){
		printf("Element (%d)\n", ptr->data->tile[0][0]);
   		ptr = ptr->next;
   }
   
}




//TODO REMOVE
// Move* top(list *sList){
// 	//assert(sList->start != NULL);
// 	if(sList == NULL || sList->head == NULL)
// 		return NULL;
// 	return sList->head->data;
// }