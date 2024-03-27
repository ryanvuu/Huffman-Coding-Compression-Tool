#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "filerw.h"
#include "freq.h"
#include "llist.h"

int main (int argc, char *argv[]) {
	int i;
	int in_file, out_file;
	/* Flag to indicate if input/output is stdin/stdout or not */
	int is_stdin, is_stdout;
	/* The total number of characters in the original file */
	uint8_t ascii;
	uint32_t freq;
	struct stat file_info;
	Node *tree;
	LinkedList *llst;
	FrequencyTable *freq_table; 
	/* Input taken from stdin and output goes to stdout */
	if (argc == 1) {
		in_file = fileno(stdin);
		is_stdin = 1;
		out_file = fileno(stdout);
		is_stdout = 1;
	}
	/* Input taken from stdin if file name is "-" 
	 * Output goes to stdout regardless */
	else if (argc == 2) {
		if (strcmp("-", argv[1]) == 0) {
			in_file = fileno(stdin);
			is_stdin = 1;
		}
		else {
			/* Opens input file in read only mode */
			in_file = open(argv[1], O_RDONLY);
			/* open returns -1 on error */
			if (in_file == -1) {
				perror(argv[1]);
				exit(EXIT_FAILURE);
			}
			is_stdin = 0;
		}
		out_file = fileno(stdout);
		is_stdout = 1;
	}
	/* Output goes to the outfile */
	else if (argc == 3) {
		if (strcmp("-", argv[1]) == 0) {
			in_file = fileno(stdin);
			is_stdin = 1;
		}
		else {
			in_file = open(argv[1], O_RDONLY);
			/* open returns -1 on error */
			if (in_file == -1) {
				perror(argv[1]);
				exit(EXIT_FAILURE);
			}
			is_stdin = 0;
		}
		/* Opens output file for writing.
		 * O_CREAT for creating the file if it doens't exist 
		 * O_TRUNC for clearing it if already exists 
		 * S_IRWXU gives the user read, write, and execute perms. */
		out_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
		/* Error handling for output file goes here */
		is_stdout = 0;
	}
	/* Print usage and exit */
	else {
		fprintf(stderr, "usage: %s [ ( infile | - ) [ outfile ] ]\n", 
				argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Get the file size. Size will be stored in st_size attribute of 
	 * stat struct that is was assigned to size_buffer */
	if (fstat(in_file, &file_info)) {
		perror("fstat");
	}
	/* Empty file */
	if (file_info.st_size == 0) {
		if (!is_stdin) {
			close(in_file);
		}
		if (!is_stdout) {
			close(out_file);
		}
		exit(EXIT_SUCCESS);
	}

	freq_table = makeFreqTable();
	/* Start reading the header */
	/* Read the first byte which is the number of unique characters - 1 */
	read(in_file, &freq_table->unique_count, sizeof(uint8_t));
	
	/* Increment unique_count by 1 */
	freq_table->unique_count += 1;
	
	/* Reconstruct the frequency table based on the header. Number of
	 * times looped is based on the unique_count. Once this loop finishes,
	 * the entire header has been read and the file pointer is at the 
	 * beginning of the body. */
	for (i = 0; i < freq_table->unique_count; i++) {
		/* The header follows the sequence: 
		 * - character: an unsigned integer of size 1 byte
		 * - frequency: an unsigned integer of size 4 bytes */
		/* Gets the character from the header */
		read(in_file, &ascii, sizeof(uint8_t));
		/* Gets the frequency of the char. just read from header
		 * Puts it into count variable first so that it can be
		 * converted to network byte order */
		read(in_file, &freq, sizeof(uint32_t));
		freq = htonl(freq);
		freq_table->freq[ascii] = freq;
		freq_table->count += freq;
	}

	/* Start regenerating the tree */
	/* Build a linked list from the frequency table */
	llst = createList(freq_table);

	/* Build the tree from the linked list */
	tree = buildTree(llst);

	/* Case where there is only one total character */
	if (freq_table->count == 1) {
		write(out_file, &tree->ascii, sizeof(char));
		exit(EXIT_SUCCESS);
	}

	/* Generate codes for each present character */
	freq_table->codes = genCodes(tree, freq_table->codes, "");

	/* Traverse tree to decode the encoded file */
	decode(in_file, out_file, tree, freq_table);

	/* Close files */
	if (!is_stdin) {
		close(in_file);
	}
	if (!is_stdout) {
		close(out_file);
	}
	
	/* Free allocated memory */
	ftableDestroy(freq_table);
	treeDestroy(tree);
	free(llst);

	return 0;
}

