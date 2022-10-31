/*
 -------------------------------------
 File:    utilities.h
 Project: Huffman TXT Compressor
 -------------------------------------
 Author:	Roy Ceyleon
 Version:	2020-11-20
 -------------------------------------
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

void prependStr(char *str, const char *c);
char* dec2bin(int n);
void stringSlice(const char *str, char *buffer, size_t start, size_t end);

#endif /* UTILITIES_H_ */
