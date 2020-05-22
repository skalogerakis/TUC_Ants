#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO data is int type. Change after test that works fine

typedef struct LinkedList LinkedList;

struct LinkedList
{
	Move* data;
	struct LinkedList *next;
	struct LinkedList *head;
};



// struct node *head;
// struct node *current;

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

// void deleteList(LinkedList *myList){
// 	LinkedList *ptrNext;

//    	if(myList->head == NULL || myList == NULL){
//    		//printf("List is empty\n");
//    		return;
//    	}

//    	while(myList->head != NULL){
// 		ptrNext = myList->head->next;
// 		//free(myList->head);
//    		myList->head->next = ptrNext;
//    	}
//    	free(myList);

//    	//printf("List deleted successfully\n");
// }


//Another implementaion. Did not help much
  /* Function to delete the entire linked list */
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

void emptyList(LinkedList *myList){
	LinkedList *ptrNext;

   	if(myList->head == NULL || myList == NULL){
   		printf("List is empty\n");
   		return;
   	}

   	while(myList->head != NULL){
		ptrNext = myList->head->next;
		//free(myList->head);
   		myList->head = ptrNext;
   	}

   	//printf("List deleted successfully\n");
}


void addElement(LinkedList *myList ,Move* data){
	LinkedList *newElem = (LinkedList*)malloc(sizeof(LinkedList));

	newElem->data = data;

	// if(myList == NULL){

	// }

	newElem->next = myList->head;
	myList->head = newElem; 
	//free(newElem);
}

// Move* removeFirst(LinkedList *myList ){
// 	printf("Remove First Element\n");

// 	//printAvailableMoves(myList);
// 	if(myList == NULL){
// 		printf("List empty\n");
// 		free(myList);
// 		return NULL;
// 	}

// 	LinkedList *initNode = myList->head;

// 	Move* returnData = malloc(sizeof(Move));

// 	//returnData = myList->head->data;
// 	memcpy(returnData,myList->head->data, sizeof(Move));

// 	// if(initNode == NULL){
// 	// 	printf("List empty\n");
// 	// 	free(initNode);
// 	// 	return NULL;
// 	// }

// 	myList->head = myList->head->next;
// 	free(initNode);

// 	return returnData;
// }

Move* removeFirst(LinkedList *myList ){
	//printf("Remove First Element\n");

	//printAvailableMoves(myList);
	if(myList == NULL || myList->head == NULL){
		//printf("List empty\n");
		return NULL;
	}

	Move* returnMove = malloc(sizeof(Move));

	LinkedList *temp = myList->head;

	memmove(returnMove,temp->data,sizeof(Move));

	myList->head = myList->head->next;

	//printf("INIT PLACE %d %d\n", tempMoveLeft->tile[0][0], tempMoveLeft->tile[1][0]);

	//printf("Remove element starting (%d,%d) and next head (%d,%d)\n", );

	// returnMove = temp->data;
	//memmove(returnMove,temp->data,sizeof(Move));
	//free(temp);

	return returnMove;
}
