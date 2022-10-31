# Huffman-TXT-Compressor
An implementation of the Huffman Tree algorithm to compress and decompress .txt files.

 ***THIS COMPRESSION IS NOT OPTIMAL FOR COMPRESSING .TXT FILES UNDER 250 BYTES, AS THE SAVINGS ARE NEGLIGIBLE OR NONEXISTENT.***

### ENCODING USAGE:
``./huffman encode <input file> <output file>``

### DECODING USAGE:
``./huffman decode <input file> <output file>``

## KNOWN LIMITATIONS
 - File to be compressed cannot contain more than 4,294,967,295 characters in total.

## INFOMATION ABOUT COMPRESSED FILE HEADER:
* First 4 bytes store the number of total characters found in the original file
* The next 5-6 bytes store the number of unique characters found in the uncompressed file
* The next 7-8 bytes store the length of the Huffman encoded binary string
* An arbitrry length of bytes following 8th byte contain frequency table
* Following the frequency table is the Huffman encoded binary string
