/*
 -------------------------------------
 File:    huffman.c
 Project: Huffman TXT Compressor
 -------------------------------------
 Author:	Roy Ceyleon
 Version:	2020-11-30
 -------------------------------------
 ***THIS COMPRESSION IS NOT OPTIMAL FOR COMPRESSING .TXT FILES UNDER 250 BYTES, AS THE SAVINGS ARE NEGLIGIBLE OR NONEXISTENT.***


 ENCODING USAGE: ./huffman encode <input file> <output file>
 DECODING USAGE: ./huffman decode <input file> <output file>

 KNOWN LIMITATIONS
 - File to be compressed cannot contain more than 4,294,967,295 characters in total.

 INFOMATION ABOUT COMPRESSED FILE HEADER:
 - First 4 bytes store the number of total characters found in the original file
 - The next 5-6 bytes store the number of unique characters found in the uncompressed file
 - The next 7-8 bytes store the length of the Huffman encoded binary string
 - An arbitrry length of bytes following 8th byte contain frequency table
 - Following the frequency table is the Huffman encoded binary string

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include "tree.h"
#include "pQueue.h"
#include "utilities.h"

//Debug Setting
#define DEBUG_MODE 1 //(0 - Disable Debugging), (1 - Enable Debugging)

//Macro Definitions
#define MAX_CHARS	256
#define BUFFER_LIMIT pow(2,15) //set to a higher power of 2 to handle txt files larger than 80 MB

//Global Variables
char char_list[MAX_CHARS] = { 0 };
int char_count[MAX_CHARS] = { 0 };
char char_encoding[MAX_CHARS][100] = { { '\0' } };
unsigned int total_char_count = 0;
short unique_char_count = 0;
char *inputFileName = NULL;
char *outputFileName = NULL;
unsigned int bit_len = 0;

//Function Declarations
void getCharCounts(char *msg);
void analyse_file();
void printBT(BT *bt);
void printAnalysis();
void getCharEncoding(TNode *node, char *str);
void _getCharEncoding(TNode *node, TNode *next_node, char *str);
bool createEncodedFile();
bool encodeFile(char *in, char *out);
bool decodeFile(char *in, char *out);

/* Main Function */
int main(int argc, char **argv) {
	if (argc < 4 || argc > 4) {
		printf("ENCODING USAGE: ./huffman encode <input file> <output file>\n");
		printf("DECODING USAGE: ./huffman decode <input file> <output file>\n");
		return 1;
	}

#if DEBUG_MODE == 1
	clock_t begin = clock();
#endif

	bool success = false;

	if (strcmp(argv[1], "encode") == 0) {
		if (strcmp(argv[2], argv[3]) == 0) {
			printf("ERROR: Input file same as Output file.");
			return 0;
		}
		success = encodeFile(argv[2], argv[3]);

#if DEBUG_MODE == 0
		printf("ENCODE[%s]->%s\n", argv[2], argv[3]);
		printf("ENCODING %s\n", success ? "SUCCESSFUL" : "FAILED");
#endif

	} else if (strcmp(argv[1], "decode") == 0) {
		if (strcmp(argv[2], argv[3]) == 0) {
			printf("ERROR: Input file same as Output file.");
			return 0;
		}
		success = decodeFile(argv[2], argv[3]);

#if DEBUG_MODE == 0
		printf("DECODE[%s]->%s\n", argv[2], argv[3]);
		printf("DECODING %s\n", success ? "SUCCESSFUL" : "FAILED");
#endif

	} else
		printf("USAGE ERROR: Invalid Arguments");

#if DEBUG_MODE == 1
	clock_t end = clock();
	printf("Execution Time: %lf sec.", (double) (end - begin) / CLOCKS_PER_SEC);
#endif

	return 0;
}

/* Function to Decode File*/
bool decodeFile(char *in, char *out) {
	inputFileName = malloc(sizeof(in));
	outputFileName = malloc(sizeof(out));
	strcpy(inputFileName, in);
	strcpy(outputFileName, out);

	FILE *iFile = NULL;
	FILE *oFile = NULL;

	// Parse the header
	char c = 0;
	int count = 0;
	char *bitstring = NULL;
	if ((iFile = fopen(inputFileName, "rb")) != NULL) {
		fread(&total_char_count, sizeof(total_char_count), 1, iFile);
		fread(&unique_char_count, sizeof(unique_char_count), 1, iFile);
		fread(&bit_len, sizeof(bit_len), 1, iFile);

		if (total_char_count < 1 || unique_char_count < 1 || bit_len < 1) {
			fclose(iFile);
			return false;
		}

#if DEBUG_MODE == 1
		printf("----HEADER INFORMATION----\n");
		printf("Total Chars: %d\n", total_char_count);
		printf("Unique Chars: %d\n", unique_char_count);
		printf("Bit Length: %d\n", bit_len);
#endif

		for (int i = 0; i < unique_char_count; i++) {
			fread(&c, sizeof(char), 1, iFile);
			fread(&count, sizeof(unsigned int), 1, iFile);
			char_count[(int) c] = count;
		}

		//Create tree nodes with symbols and their respective weights, and insert into Queue
		PriorityQueue *pQ = createPQueue();
		for (int i = 0; i < sizeof(char_count); i++) {
			if (char_count[i] > 0) {
				TNode *new_node = (TNode*) malloc(sizeof(TNode));
				new_node->parent = NULL;
				new_node->left = NULL;
				new_node->right = NULL;
				new_node->symbol = (char) i;
				new_node->weight = char_count[i];
				enqueuePQueue(pQ, new_node);
			}
		}

		//Build Huffman Tree
		while (pQ->size > 1) {
			TNode *n1 = dequeuePQueue(pQ);
			TNode *n2 = dequeuePQueue(pQ);
			TNode *new_node = (TNode*) malloc(sizeof(TNode));
			new_node->left = n1;
			new_node->right = n2;
			n1->parent = new_node;
			n2->parent = new_node;
			new_node->weight = n1->weight + n2->weight;
			new_node->symbol = '\0';
			enqueuePQueue(pQ, new_node);
		}

		BT *bt = (BT*) malloc(sizeof(BT));
		initializeBT(bt, dequeuePQueue(pQ));

#if DEBUG_MODE == 1
		printBT(bt);
		//	printf("Huffman Binary Encoding:\n");
		//	printf("%s\n", bitstring);
#endif

		//Traverse tree and decode the file
		if ((oFile = fopen(outputFileName, "wb")) != NULL) {
			unsigned int processed = 0;
			bitstring = (char*) calloc(UINT_MAX, sizeof(char));
			unsigned char ch;
			size_t check;

			//decode encoded binary in chunks of 1024 bits and write
			while ((check = fread(&ch, sizeof(char), 1, iFile)) > 0) {
				strcat(bitstring, dec2bin((unsigned int )ch));

				if (strlen(bitstring) >= 1024) {
					int written = 0;
					TNode *current = bt->root;
					for (int i = 0; i < strlen(bitstring); i++) {
						processed++;
						if (bitstring[i] == '0')
							current = current->left;
						else
							current = current->right;

						if (current->symbol != '\0') {
							fwrite(&current->symbol, 1, sizeof(char), oFile);
							current = bt->root;
							if (++written > 10) {
								stringSlice(bitstring, bitstring, i + 1,
										strlen(bitstring));
								break;
							}
						}

					}
				}

			}

			//write any leftover bits
			TNode *current = bt->root;
			for (int i = 0; i < bit_len - processed; i++) {
				if (bitstring[i] == '0')
					current = current->left;
				else
					current = current->right;

				if (current->symbol != '\0') {
					fwrite(&current->symbol, 1, sizeof(char), oFile);
					current = bt->root;
				}
			}

		} else {
			free(bt);
			free(bitstring);
			return false;
		}

#if DEBUG_MODE == 1
		printf("Generated Uncompressed File: %s\n", outputFileName);
#endif
		fclose(iFile);
		fclose(oFile);
	} else
		return false;

	return true;
}

/* Function to Encode File*/
bool encodeFile(char *in, char *out) {

	//set filenames for global access
	inputFileName = malloc(sizeof(in));
	outputFileName = malloc(sizeof(out));
	strcpy(inputFileName, in);
	strcpy(outputFileName, out);

	//Analyse and Prints results
	analyse_file();

#if DEBUG_MODE == 1
	printAnalysis();
	printf("\n");
#endif

	//Create tree nodes with symbols and their respective weights, and insert into Queue
	PriorityQueue *pQ = createPQueue();
	for (int i = 0; i < sizeof(char_count); i++) {
		if (char_count[i] > 0) {
			TNode *new_node = (TNode*) malloc(sizeof(TNode));
			new_node->parent = NULL;
			new_node->left = NULL;
			new_node->right = NULL;
			new_node->symbol = (char) i;
			new_node->weight = char_count[i];
			enqueuePQueue(pQ, new_node);
		}
	}

	//Build Huffman Tree
	while (pQ->size > 1) {
		TNode *n1 = dequeuePQueue(pQ);
		TNode *n2 = dequeuePQueue(pQ);
		TNode *new_node = (TNode*) malloc(sizeof(TNode));
		new_node->left = n1;
		new_node->right = n2;
		n1->parent = new_node;
		n2->parent = new_node;
		new_node->weight = n1->weight + n2->weight;
		new_node->symbol = '\0';
		enqueuePQueue(pQ, new_node);
	}

	//Print BT
	BT *bt = (BT*) malloc(sizeof(BT));
	initializeBT(bt, dequeuePQueue(pQ));

#if DEBUG_MODE == 1
	printBT(bt);
#endif

	//Get New Char Encodings
	char str[50] = "";
	getCharEncoding(bt->root, str);

#if DEBUG_MODE == 1
	printf("\n");
	printf("----CHAR ENCODINGS----\n");

	for (int i = 0; char_list[i] != '\0'; i++)
		printf("%c - %s\n", char_list[i] == '\n' ? '|' : char_list[i],
				char_encoding[(int) char_list[i]]);

#endif

	return createEncodedFile();
}

//Builds the encoded file
bool createEncodedFile() {
	FILE *file = NULL;
	char *txt_buffer = 0;
	unsigned long txt_length = 0;

	char *bitstring = (char*) calloc(UINT_MAX, sizeof(char));
	if (bitstring == NULL)
		return false;

	if ((file = fopen(inputFileName, "r")) != NULL) {

		if (getc(file) == EOF) { //empty file check
			printf("<Empty File>\n");
			fclose(file);
			exit(0);
		}

		//retrieve the size of the txt and allocate memory for buffer
		fseek(file, 0, SEEK_END);
		txt_length = ftell(file);
		txt_buffer = malloc(txt_length);
		fseek(file, 0, SEEK_SET);

		//read entire file to buffer
		if (txt_buffer)
			fread(txt_buffer, 1, txt_length, file);

		//close the file
		fclose(file);
	}

	if ((file = fopen(outputFileName, "wb")) != NULL) {
		// write header - total char count and unique char count
		fwrite(&total_char_count, 1, sizeof(total_char_count), file);
		fwrite(&unique_char_count, 1, sizeof(unique_char_count), file);

		//write bit string length to header
		for (int i = 0; txt_buffer[i] != '\0'; i++)
			bit_len += strlen(char_encoding[(int) txt_buffer[i]]);
		fwrite(&bit_len, 1, sizeof(bit_len), file);

		//write the frequency table to file
		for (int i = 0; i < MAX_CHARS; i++) {
			if (char_count[i] > 0) {
				fwrite(&i, 1, sizeof(char), file);
				fwrite(&char_count[i], 1, sizeof(unsigned int), file);
			}
		}

		// construct bitstring and process in 256 bit chunks
		char *bitstring = (char*) calloc(UINT_MAX, sizeof(char));
		for (int i = 0; txt_buffer[i] != '\0'; i++) {

			strcat(bitstring, char_encoding[(int ) txt_buffer[i]]);

			if (strlen(bitstring) >= BUFFER_LIMIT) {

				char *temp = (char*) calloc(UINT_MAX, sizeof(char));

				stringSlice(bitstring, temp,
						strlen(bitstring) - strlen(bitstring) % 8,
						strlen(bitstring));
				stringSlice(bitstring, bitstring, 0,
						strlen(bitstring) - strlen(bitstring) % 8);

				//write the Huffman encoding string to file
				for (int k = 0; k < strlen(bitstring) - 7; k += 8) {
					short int bin = 0;
					for (int i = 0; i < 8; i++) {
						if (bitstring[k + i] == '1')
							bin += pow(2, (8 - i - 1));
					}

					fwrite(&bin, 1, 1, file);

				}
				if (strlen(temp) > 0) {

					strcpy(bitstring, temp);
					free(temp);
				} else {
					bitstring = (char*) calloc(UINT_MAX, sizeof(char));
				}

			}
		}

		//write any leftovers bits
		for (int k = 0; k < strlen(bitstring); k += 8) {
			short int bin = 0;
			for (int i = 0; i < 8; i++) {
				if (bitstring[k + i] == '1')
					bin += pow(2, (8 - i - 1));
			}
			fwrite(&bin, 1, 1, file);
		}

		//close the file and clean up
		fclose(file);

	} else {
		free(txt_buffer);
		free(bitstring);
		return false;
	}

#if DEBUG_MODE == 1
	//binary string that is written to file
//	printf("\nHuffman Binary Encoding of %s:\n", inputFileName);
//	printf("%s\n", bitstring);
	printf("Bits: %d\n", bit_len);
	printf("Generated Compressed File: %s\n", outputFileName);
#endif

	free(bitstring);
	free(txt_buffer);
	return true;

}

/* Function that retrieves the binary string of any given char in the tree */
void getCharEncoding(TNode *node, char *str) {
	if (node == NULL)
		return;

	if (node->left == NULL && node->right == NULL) {
		_getCharEncoding(node, node, str);
	} else {
		getCharEncoding(node->left, str);
		getCharEncoding(node->right, str);
	}
}

/* Auxiliary Function for getCharEncoding */
void _getCharEncoding(TNode *node, TNode *next_node, char *str) {
	if (next_node->parent != NULL) {
		if (next_node == next_node->parent->left)
			prependStr(str, "0");
		if (next_node == next_node->parent->right)
			prependStr(str, "1");
		_getCharEncoding(node, next_node->parent, str);
	} else {
		strcpy(char_encoding[(int )node->symbol], str);
		strcpy(str, "");
	}
}

/* Prints the Constructed Huffman Tree */
void printBT(BT *bt) {
	printf("----HUFFMAN TREE----\n");
	printf("Note: _ nodes refer to null nodes.\n");
	Queue *Q = createQueue();
	enqueueQueue(Q, bt->root);

	int j = 1;
	while (Q->size > 0) {
		TNode *node = dequeueQueue(Q);

		if (node->left != NULL) {
			enqueueQueue(Q, node->left);
		}
		if (node->right != NULL) {
			enqueueQueue(Q, node->right);
		}
		if (j > 5) {
			printf("\n");
			j = 1;
		}
		printf("(%-2c - %2d)\t",
				node->symbol == '\0' ? '_' :
				node->symbol == '\n' ? '*' : node->symbol, node->weight);
		j++;
	}
	printf("\n");
	printf("Tree contains %d nodes.\n", bt->size);
}

/* Tester Function for Priority Queue */
int pQTest() {

	PriorityQueue *pQ = createPQueue();

	int nodes = rand() % 10;
	for (int i = 0; i < nodes; i++) {
		TNode *new_node = (TNode*) malloc(sizeof(TNode));
		new_node->weight = rand() % 500;
		enqueuePQueue(pQ, new_node);
	}

	QNode *ptr = pQ->front;
	while (ptr != NULL) {
		printf("%d\n", ptr->data->weight);
		ptr = ptr->next;
	}

	printf("PQueue Size: %d\n", pQ->size);

	while (pQ->front != NULL) {
		TNode *node = dequeuePQueue(pQ);
		if (node != NULL)
			printf("%d\n", node->weight);
	}

	printf("PQueue Size: %d\n", pQ->size);

	return 0;
}

/* File Analysis Function */
void analyse_file() {
	FILE *file = NULL;
	char *txt_buffer = 0;
	unsigned long txt_length = 0;

	if ((file = fopen(inputFileName, "r")) != NULL) {

		if (getc(file) == EOF) { //empty file check
			printf("<Empty File>\n");
			fclose(file);
			exit(0);
		}

		//retrieve the size of the txt and allocate memory for buffer
		fseek(file, 0, SEEK_END);
		txt_length = ftell(file);
		txt_buffer = malloc(txt_length);
		fseek(file, 0, SEEK_SET);

		//read entire file to buffer
		if (txt_buffer)
			fread(txt_buffer, 1, txt_length, file);

		//close the file
		fclose(file);

		getCharCounts(txt_buffer);
	}
}

/* Prints the results of File Analysis */
void printAnalysis() {
	printf("----PRINT ANALYSIS----\n");
	for (int i = 0, j = 1; char_list[i] != '\0'; i++, j++) {
		if (j > 10) {
			printf("\n");
			j = 1;
		}
		printf("[%c:%d]\t", char_list[i] == '\n' ? '|' : char_list[i],
				char_count[(int) char_list[i]]);
	}
	printf("\n");
	printf("Total Characters: %d\n", total_char_count);
	printf("Unique Characters: %d\n", unique_char_count);
}

/* Retrieves relevant information for the construction of the Huffman Tree */
void getCharCounts(char *msg) {

	for (int i = 0; msg[i] != '\0'; i++) {
		//check and add character to char list
		total_char_count++;
		for (int k = 0; k < MAX_CHARS; k++) {
			// character already in char list, skip
			if (char_list[k] == msg[i])
				break;
			// character not found in char list, so add it
			if (char_list[k] == 0) {
				char_list[k] = msg[i];
				unique_char_count++;
				break;
			}
		}
		//increment or initialize char count of the character
		char_count[(int) msg[i]]++;
	}
}
