#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "freq.h"
#include "llist.h"
#include "filerw.h"

int main(int argc, char *argv[]) {
	/* The size of the input file */
	int file_size;
	int in_file, out_file;
	/* Flag to indicate if output is stdout or not */
	int is_stdout;
	/* Buffer to hold the size of a file after using fstat */
	struct stat size_buffer;
	Node *tree;
	LinkedList *llst;
	FrequencyTable *freq_table; 

	/* Output goes to stdout */
	if (argc == 2) {
		/* Opens input file in read only mode */
		in_file = open(argv[1], O_RDONLY);
		/* open returns -1 on error */
		if (in_file == -1) {
			perror(argv[1]);
			exit(EXIT_FAILURE);
		}
		out_file = fileno(stdout);
		is_stdout = 1;
	}
	/* Output goes to the outfile */
	else if (argc == 3) {
		in_file = open(argv[1], O_RDONLY);
		/* open returns -1 on error */
		if (in_file == -1) {
			perror(argv[1]);
			exit(EXIT_FAILURE);
		}
		/* Opens output file for writing.
		 * O_CREAT for creating the file if it doens't exist 
		 * O_TRUNC for clearing it if already exists 
		 * S_IRWXU gives the user read, write, and execute perms. */
		out_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
		/* Error handling not needed for output file because if it 
		 * doesn't exist, it will be created */
		is_stdout = 0;
	}
	/* Print usage and exit */
	else {
		fprintf(stderr, "usage %s infile [ outfile ]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Get the file size. Size will be stored in st_size attribute of 
	 * stat struct that was assigned to size_buffer */
	if (fstat(in_file, &size_buffer)) {
		perror("fstat");
	}
	file_size = size_buffer.st_size;
	
	/* Empty file */
	if (file_size == 0) {
		close(in_file);
		if (!is_stdout) {
			close(out_file);
		}
		exit(EXIT_SUCCESS);
	}

	/* Make a frequency table and get each character's frequency 
	 * from the file */
	freq_table = makeFreqTable();
	genFreq(in_file, file_size, freq_table);
	/* Set the file pointer back to the beginning since genFreq moved
	 * it to the end */
	if(lseek(in_file, 0, SEEK_SET) == -1) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}
	/* Construct a linked list from the frequencies */
	llst = createList(freq_table);

	/* Combine all nodes and Construct the tree from the linked list 
	 * then traverse the tree to get each character's hcode */
	tree = buildTree(llst);
	freq_table->codes = genCodes(tree, freq_table->codes, "");
	
	/* Write header to output */
	makeHeader(out_file, freq_table);
	/* Write body to output */
	makeBody(in_file, file_size, out_file, freq_table);
	
	/* Close input file */
	close(in_file);
	/* Close output file only if output wasn't stdout */
	if (!is_stdout) {
		close (out_file);
	}
	/* Free all dynamically allocated structs */
	ftableDestroy(freq_table);
	treeDestroy(tree);
	free(llst);
	return 0;
}

