/*
 -------------------------------------
 File:    pQueue.c
 Project: Huffman TXT Compressor
 -------------------------------------
 Author:	Roy Ceyleon
 Version:	2020-11-20
 -------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "tree.h"
#include "pQueue.h"

PriorityQueue* createPQueue() {
	PriorityQueue *pQ = (PriorityQueue*) malloc(sizeof(PriorityQueue));
	pQ->front = NULL;
	return pQ;
}

Queue* createQueue() {
	Queue *Q = (Queue*) malloc(sizeof(Queue));
	Q->front = NULL;
	return Q;
}

void enqueuePQueue(PriorityQueue *pQ, TNode *node) {
	QNode *new_node = (QNode*) malloc(sizeof(QNode));
	QNode *ptr = NULL;
	QNode *prev_ptr = NULL;

	new_node->data = node;
	new_node->next = NULL;

	if (pQ->front == NULL) {
		pQ->front = new_node;
	} else {
		ptr = pQ->front;

		while (ptr != NULL && new_node->data->weight > ptr->data->weight) {
			prev_ptr = ptr;
			ptr = ptr->next;
		}

		if (ptr == pQ->front) {
			new_node->next = ptr;
			pQ->front = new_node;
		} else if (ptr == NULL) {
			prev_ptr->next = new_node;
		} else {
			new_node->next = ptr;
			prev_ptr->next = new_node;
		}
	}

	pQ->size++;

}

void enqueueQueue(Queue *Q, TNode *node) {
	QNode *new_node = (QNode*) malloc(sizeof(QNode));
	QNode *ptr = NULL;
	QNode *prev_ptr = NULL;

	new_node->data = node;
	new_node->next = NULL;

	if (Q->front == NULL) {
		Q->front = new_node;
	} else {
		ptr = Q->front;

		while (ptr != NULL) {
			prev_ptr = ptr;
			ptr = ptr->next;
		}

		new_node->next = ptr;
		prev_ptr->next = new_node;
	}

	Q->size++;

}

TNode* dequeuePQueue(PriorityQueue *pQ) {

	// Empty PQueue, do nothing, return NULL
	if (pQ->front == NULL)
		return NULL;

	QNode *temp = pQ->front;
	TNode *node = pQ->front->data;
	pQ->front = pQ->front->next;
	pQ->size--;
	free(temp);

	return node;
}

TNode* dequeueQueue(Queue *Q) {

	// Empty PQueue, do nothing, return NULL
	if (Q->front == NULL)
		return NULL;

	QNode *temp = Q->front;
	TNode *node = Q->front->data;
	Q->front = Q->front->next;
	Q->size--;
	free(temp);

	return node;
}
