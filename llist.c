#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "llist.h"

/* Creates a node */
Node *createNode(int ascii, int freq) {
	Node *node = (Node *)malloc(sizeof(Node));
	if (!node) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	node->ascii = ascii;
	node->freq = freq;
	node->next = NULL;
	node->prev = NULL;
	node->left = NULL;
	node->right = NULL;
	return node;
}

/* Creates a linked list from a frequency table. */
LinkedList *createList(FrequencyTable *freq_table) {
	int i;
	Node *node;
	LinkedList *llst = (LinkedList *)malloc(sizeof(LinkedList));
	if (!llst) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	llst->head = NULL;
	llst->size = 0;

	for (i = 0; i < freq_table->size; i++) {
		if (freq_table->freq[i] > 0) {
			node = createNode(i, freq_table->freq[i]);
			lstInsert(llst, node);
		}
	}
	return llst;
}

/* Insert node into the linked list in order of ascending frequency */
void lstInsert(LinkedList *llst, Node *node) {
	Node *temp;
	/* Node is the head if llst is empty */
	if (llst->head == NULL) {
		llst->head = node;
	}
	/* Node's frequency is less than the frequency of the head */
	else if (node->freq < llst->head->freq) {
		node->next = llst->head;
		llst->head->prev = node;
		llst->head = node;
	}
	/* Otherwise add node in ascending order. */
	else {
		temp = llst->head;
		while (temp != NULL) {
			/* Frequency of node to be inserted is less than 
			 * existing node */
			if (node->freq < temp->freq) {
				node->prev = temp->prev;
				node->next = temp;
				temp->prev->next = node;
				temp->prev = node;
				break;
			}
			/* Frequency of node is the same as existing node.
			 * Compare ASCII values */
			else if (node->freq == temp->freq) {
				/* This tie handling is only when building the
				 * initial linked list */
				if (node->ascii < temp->ascii) {
					node->prev = temp->prev;
					node->next = temp;
					temp->prev->next = node;
					temp->prev = node;
					break;
				}
			}
			/* Largest frequency or tied for longest w/ biggest
			 * ASCII value. */
			if (temp->next == NULL) {
				node->prev = temp;
				temp->next = node;
				break;
			}
			temp = temp->next;
		}
	}
	llst->size += 1;	
}

/* Insertion function during tree building */
void huffLstInsert(LinkedList *llst, Node *node) {
	Node *temp;
	/* Node is the head if llst is empty */
	if (llst->head == NULL) {
		llst->head = node;
	}
	/* Node's frequency is less than or equal to the 
	 * frequency of the head */
	else if ( (node->freq < llst->head->freq) 
			|| (node->freq == llst->head->freq) ) {
		node->next = llst->head;
		llst->head->prev = node;
		llst->head = node;
	}
	/* Otherwise add node in ascending order. */
	else {
		temp = llst->head;
		while (temp != NULL) {
			/* Frequency of node to be inserted is less than 
			 * existing node */
			if (node->freq < temp->freq) {
				node->prev = temp->prev;
				node->next = temp;
				temp->prev->next = node;
				temp->prev = node;
				break;
			}
			/* Frequency of node is the same as existing node.
			 * Compare ASCII values */
			else if (node->freq == temp->freq) {
				node->prev = temp->prev;
				node->next = temp;
				temp->prev->next = node;
				temp->prev = node;
				break;
			}
			/* Largest frequency or tied for longest w/ biggest
			 * ASCII value. */
			if (temp->next == NULL) {
				node->prev = temp;
				temp->next = node;
				break;
			}
			temp = temp->next;
		}
	}
	llst->size += 1;	
}


/* Removes the first node from a linked list and returns it */
Node *removeNode(LinkedList *llst) {
	if (llst == NULL) {
		return NULL;
	}
	Node *temp = llst->head;
	/* Head is the only node in the linked list */
	if (llst->size == 1) {
		llst->head = NULL;
	}
	else {
		llst->head = llst->head->next;
		temp->next->prev = NULL;
		temp->next = NULL;
	}
	llst->size -= 1;
	return temp;
}

void printList(LinkedList *llst) {
	Node *temp = llst -> head;
	while (temp != NULL) {
		if (temp ->next == NULL) {
			printf("(%c)%d <-> NULL\n", temp->ascii, temp->freq);
			}
		else {
			printf ("(%c)%d <-> ", temp->ascii, temp->freq);
		}
		temp = temp ->next;
	}
}

/* Combines two nodes into one */
Node *buildTree(LinkedList *llst) {
	Node *combined, *left, *right;
	int freqSum;
	while (llst->size != 1) {
		left = removeNode(llst);
		right = removeNode(llst);
		freqSum = left->freq + right->freq;
		/* Combined node arbitrarily has its ascii the same as the left 
		 * child's ascii. This does not affect anything, though, since 
		 * this is never utilized */
		combined = createNode(left->ascii, freqSum);
		combined->left = left;
		combined->right = right;
		huffLstInsert(llst, combined);
	}
	return llst->head;
}

/* Generates codes of each character on the tree into the array of 
 * char pointers in the frequency table. Takes in an empty string 
 * initially. */
char **genCodes(Node *tree, char **codes, char *build_code) {
	/* Every time temp code is to be built, increase its size by 2
	 * because you need +1 for either another 0 or 1 and +1 for the
	 * null character. */
	char *final_code;
	char temp_code[strlen(build_code) + 2];
	strcpy(temp_code, build_code);

	/* Go left until node has no left node */
	if (tree->left) {
		sprintf(temp_code, "%s0", build_code); 
		genCodes(tree->left, codes, temp_code);
	}
	/* Go right until node has no right node */
	if (tree->right) {
		sprintf(temp_code, "%s1", build_code); 
		genCodes(tree->right, codes, temp_code);
	}
	/* Leaf node encountered, copy temp code into codes using strdup
	 * instead of strcpy because strdup mallocs */
	if (!(tree->left && tree->right)) {
		final_code = strdup(temp_code);
		codes[tree->ascii] = final_code;
	}
	return codes;
}

void treeDestroy(Node *tree) {
	if (tree == NULL) {
		return;
	}
	if (tree->left) {
		treeDestroy(tree->left);
	}
	if (tree->right) {
		treeDestroy(tree->right);
	}
	free(tree);
}

