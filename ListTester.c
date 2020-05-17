//
// Created by skalogerakis on 17/5/20.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "client.h"

void main() {

    LinkedList* moveList = malloc(sizeof(LinkedList));

    LinkedListInitializer(moveList);

    addElement(moveList,1);
    addElement(moveList,12);
    addElement(moveList,32);
    addElement(moveList,25);
    addElement(moveList,-71);

    printList(moveList);

    removeFirst(moveList);
    removeFirst(moveList);

    addElement(moveList,1000);

    printList(moveList);

    deleteList(moveList);

    printList(moveList);
    return;
}