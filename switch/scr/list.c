#include <stdlib.h>
#include <string.h>
#include "list.h"

void addToList(List *list, char *data) {
	Node *node = malloc(sizeof(Node));
	char *str = malloc(strlen(data) + 1);
	strcpy(str, data);
	node->data = str;
	node->next = list->first;
	list->first = node;
}

void removeFromList(List *list, char *data) {
	Node *node = list->first;
	if (list->first != NULL && strcmp(list->first->data, data) == 0) {
		node = list->first;
		list->first = list->first->next;
		free(node->data);
		free(node);
	} else if (list->first != NULL) {
		node = list->first;
		while (node->next != NULL && strcmp(node->next->data, data) != 0) {
			node = node->next;
		}
		if (node->next != NULL) {
			node->next = node->next->next;
			free(node->data);
			free(node);
		}
	}
}

int isListEmpty(List *list) {
	return list->first == NULL;
}

void emptyList(List *list) {
	if (!isListEmpty(list)) {
		Node *node = list->first;
		while (node != NULL) {
			list->first = node->next;
			free(node->data);
			free(node);
			node = list->first;
		}
	}
}

List *createList() {
	List *list = malloc(sizeof(List));
	list->first = NULL;
	return list;
}

void destroyList(List *list) {
	emptyList(list);
	free(list);
}

int isInList(List *list, char *data) {
	Node *node = list->first;
	while(node != NULL) {
		if (strcmp(node->data, data) == 0) {
			return 1;
		}
		node = node->next;
	}
	return 0;
}
