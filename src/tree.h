/*
 -------------------------------------
 File:    tree.h
 Project: Huffman TXT Compressor
 -------------------------------------
 Author(s):	Roy Ceyleon
 Version:	2020-11-20
 -------------------------------------
 */

#ifndef TREE_H_
#define TREE_H_

typedef struct TNode {
	struct TNode *parent;
	struct TNode *left;
	struct TNode *right;
	char symbol;
	int weight;
} TNode;

typedef struct BT {
	TNode *root;
	int size;
} BT;

void initializeBT(BT *bt, TNode *root_node);

#endif /* TREE_H_ */
