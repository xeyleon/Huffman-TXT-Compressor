/*
 -------------------------------------
 File:    utilities.c
 Project: Huffman TXT Compressor
 -------------------------------------
 Author(s):	Roy Ceyleon
 Version:	2020-11-20
 -------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

/* Function to prepend a char to a string*/
void prependStr(char *str, const char *c) {
	size_t len = strlen(c);
	memmove(str + len, str, strlen(str) + 1);
	memcpy(str, c, len);
}

/* File to convert a given integer to a binary string */
char* dec2bin(int n) {
	int binaryNum[32];
	char *bitstring = malloc(10);
	for (int i = 0; i < sizeof(bitstring); i++)
		bitstring[i] = '\0';

	//store binary
	int i = 0;
	while (n > 0) {
		binaryNum[i] = n % 2;
		n = n / 2;
		i++;
	}

	//construct binary string
	for (int j = i - 1; j >= 0; j--) {
		if (binaryNum[j] == 1)
			strcat(bitstring, "1");
		else
			strcat(bitstring, "0");
	}

	//pad with zeroes
	while (strlen(bitstring) < 8)
		prependStr(bitstring, "0");

	return bitstring;
}

/* Returns a slice of a given string */
void stringSlice(const char *str, char *buffer, size_t start, size_t end) {
	size_t k = 0;
	for (size_t i = start; i <= end; ++i) {
		buffer[k++] = str[i];
	}
	buffer[k] = 0;
}
