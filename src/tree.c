/*
 -------------------------------------
 File:    tree.c
 Project: Huffman TXT Compressor
 -------------------------------------
 Author:	Roy Ceyleon
 Version:	2020-11-20
 -------------------------------------
 */

#include "tree.h"
#include "pQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void initializeBT(BT *bt, TNode *root_node) {
	bt->root = root_node;
	bt->size = 1;

	Queue *Q = createQueue();
	enqueueQueue(Q, bt->root);

	while (Q->size > 0) {
		TNode *node = dequeueQueue(Q);

		if (node->left != NULL) {
			enqueueQueue(Q, node->left);
			bt->size++;
		}
		if (node->right != NULL) {
			enqueueQueue(Q, node->right);
			bt->size++;
		}
	}

}
