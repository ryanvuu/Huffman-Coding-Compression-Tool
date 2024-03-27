#ifndef LLISTH
#define LLISTH
#include "freq.h"

typedef struct Node Node;
typedef struct LinkedList LinkedList;

/* Node: Represents a single element in a linked list. */
struct Node {
	/* ASCII value that the node represents. */
	unsigned int ascii;
	/* Frequency of a character represented by the node */
	int freq;
	/* Pointer to the next node in the linked list */
	Node *next;
	/* Pointer to the previous node in the linked list */
	Node *prev;
	/* Left child node */
	Node *left;
	/* Right child node */
	Node *right;
};

struct LinkedList {
	/* Head of the linked list */
	Node *head;
	/* Size of the linked list */
	unsigned int size;
};

Node *createNode(int, int);
LinkedList *createList(FrequencyTable *);
void lstInsert(LinkedList *, Node *);
void huffLstInsert(LinkedList *, Node *);
Node *removeNode(LinkedList *);
void printList(LinkedList *);
Node *buildTree(LinkedList *);
char **genCodes(Node *, char **, char *);
void treeDestroy(Node *);
#endif

