#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "freq.h"
#include "llist.h"

/* Number of bits in a byte */
#define BYTE_SIZE 8
/* Number of bits to go from four bytes to one byte */
#define FOUR_TO_ONE 24

/* Writes the header to an output file. The header contains the 
 * frequencies of each char. that appears in the input file so that
 * the tree can be re-created. 
 *
 * Paramters:
 *  fdout - A file descriptor for the output file 
 *  freq_table - A pointer to a Frequency Table 
 */
void makeHeader(int fdout, FrequencyTable *freq_table) {
	int i;
	/* represents number of unique words  - 1 */
	uint8_t num = freq_table->unique_count - 1;
	/* The ascii of a character */
	uint8_t c;
	/* The frequency corresponding to a character */
	uint32_t freq;

	/* First write num - 1 */
	/* Convert num to network byte order */
	/* htonl returns a 32 bit number so need to convert num back to
	 * 8 bits. This is done by right shifting by 24 which moves the
	 * bits into positions 0-7. */
	num = (uint8_t)(htonl(num) >> FOUR_TO_ONE);
	write(fdout, &num, sizeof(uint8_t));
	/* Then, write each char and frequency */
	for (i = 0; i < freq_table->size; i++) {
		if (freq_table->freq[i] > 0) {
			c = i;
			freq = freq_table->freq[i];
			/* Convert to network byte order */
			freq = htonl(freq);
			/* Convert c back to 8 bits */
			c = (uint8_t)(htonl(c) >> FOUR_TO_ONE);
			write(fdout, &c, sizeof(uint8_t));
			write(fdout, &freq, sizeof(uint32_t));
		}
	}
}

/* Writes a "body" to an output file. The body represents the encoded
 * input file. 
 *
 * Parameters:
 *  fdin - A file descriptor for the input file 
 *  size - The size of the input file 
 *  fdout - A file descriptor for the output file 
 *  freq_table - A pointer to a Frequency Table 
 */
void makeBody(int fdin, int size, int fdout, FrequencyTable *freq_table) {
	/* ascii of a read character */
	unsigned int c = 0;
	int i, j;
	/* Holds the number of zeros that need to be padded to a byte */
	int padding;
	/* Keeps track of how many characters have been written to out file */
	int counter = 0;
	/* The hcode corresponding to a character */
	char *str_code;
	/* The length of the hcode */
	int code_len;
	/* The literal bit representation of the hcode */
	uint8_t bit_code = 0;
	for (i = 0; i < size; i++) {
		/* Gets a character in the file */
		read(fdin, &c, 1);
		/* Gets the code belonging to the char */
		str_code = freq_table->codes[c];
		code_len = strlen(str_code);
		for (j = 0; j < code_len; j++) {
			/* Multiplying by 2 to keep track of bit position 
			 * in byte */
			bit_code = bit_code * 2;
			if (str_code[j] == '1') {
				bit_code += 1;	
			}
			counter += 1;

			/* A byte has been constructed */ 
			if (counter % BYTE_SIZE == 0) {
				/* Convert to network byte order and 
				 * convert back to 8 bits since htonl 
				 * returns a 32 bit number */
				bit_code = (uint8_t)(htonl(bit_code) >> 
							FOUR_TO_ONE);
				write(fdout, &bit_code, sizeof(uint8_t));
				counter = 0;
			}

		}
	}

	/* Check the final counter to see if padding is needed. 
	 * If it is needed, then we need to pad with zeros via left 
	 * shifting by however many bits is needed to reach a BYTE_SIZE.
	 * Then we can write this to the output file */
	if (counter > 0) {
		padding = BYTE_SIZE - counter;
		bit_code = bit_code << padding;
		/* Convert to network byte order and convert back to 
		* 8 bits since htonl returns a 32 bit number */
		bit_code = (uint8_t)(htonl(bit_code) >> FOUR_TO_ONE);
		write(fdout, &bit_code, sizeof(uint8_t));
	}
}

/* Converts a huffman encoded file into its character representation 
 * 
 * Parameters:
 *  fdin - A file descriptor for the input file
 *  fdout - A file descriptor for the output file
 *  tree - A pointer to a node that represents a huffman tree
 *  freq_table - A pointer to a Frequency Table
 */
void decode(int fdin, int fdout, Node *tree, FrequencyTable *freq_table) {
	/* Saves the top of the tree */
	Node *root = tree;
	int i;
	/* Keeps track of how many characters have been decoded */
	int counter = 0;
	/* Stores a bit based on the byte that was read from the body */
	int first_bit;
	/* Sets a constant that will be used as an operand when getting the 
	 * first bit from a byt*/
	const int bit = 1;
	uint8_t read_byte;

	/* Keep reading bytes. The loop will stop once all the 
	 * characters have been decoded. 
	 * Empty file check occurs in main. 
	 * Single character check occurs in main. */
	while (1) {
		/* Reads a byte from the body */
		read(fdin, &read_byte, sizeof(uint8_t));

		/* Traversing tree */
		for (i = 0; i < BYTE_SIZE; i++) {
			/* Left shift the read byte by 1 with rotation so 
			 * that the most significant bit becomes the least 
			 * significant bit */
			read_byte = (read_byte << 1) | 
						(read_byte >> (BYTE_SIZE - 1));

			/* Gets the first bit in the read byte */
			first_bit = read_byte & bit;
			/* Traverse left */
			if ((first_bit == 0) && (tree->left)) {
				tree = tree->left;
			}
			/* Traverse right */
			if ((first_bit == 1) && (tree->right)) {
				tree = tree->right;
			}
			
			/* Leaf node reached */
			if (!(tree->left && tree->right)) {
				/* Write char to out_file. A char is one byte, 
				 * so write one byte to the file */
				write(fdout, &tree->ascii, sizeof(uint8_t));
				counter += 1;
				/* Go back to the top of the tree */
				tree = root;
			}

			/* Check if all characters have been decoded */
			if (counter == freq_table->count) {
				return;
			}
		}
	}
}

