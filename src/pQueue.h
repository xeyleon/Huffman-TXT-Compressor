/*
 -------------------------------------
 File:    pQueue.h
 Project: Huffman TXT Compressor
 -------------------------------------
 Author: Roy Ceyleon
 Version: 2020-11-20
 -------------------------------------
 */

#ifndef PQUEUE_H_
#define PQUEUE_H_

typedef struct QNode {
	struct TNode *data;
	struct QNode *next;
} QNode;

typedef struct PriorityQueue {
	QNode *front;
	int size;
} PriorityQueue;

typedef struct Queue {
	QNode *front;
	int size;
} Queue;

PriorityQueue* createPQueue();
Queue* createQueue();

void enqueuePQueue(PriorityQueue *pQ, TNode *node);
void enqueueQueue(Queue *Q, TNode *node);

TNode* dequeuePQueue(PriorityQueue *pQ);
TNode* dequeueQueue(Queue *Q);

#endif /* PQUEUE_H_ */
