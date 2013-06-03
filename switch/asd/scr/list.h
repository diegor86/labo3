/*
 * list
 *
 *  Created on: 01/06/2013
 *      Author: oscar
 */

#ifndef LIST_H
#define LIST_H

typedef struct NodeT {
	char *data;
	struct NodeT *next;
} Node;

typedef struct {
	Node *first;
} List;

void addToList(List *list, char *data);
void removeFromList(List *list, char *data);
int isListEmpty(List *list);
void emptyList(List *list);
List *createList();
void destroyList(List *list);
int isInList(List *list, char *data);

#endif /* LIST_ */
