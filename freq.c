#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "freq.h"
#include "filerw.h"

/* Initializes a frequency table */
FrequencyTable *makeFreqTable(void) {
	int i;
	FrequencyTable *freq_table = calloc(1, sizeof(FrequencyTable));
	if (!freq_table) {
		perror("malloc FrequencyTable");
		exit(EXIT_FAILURE);
	}
	freq_table->count = 0;
	freq_table->unique_count = 0;
	freq_table->size = MAX_NUM_BYTES;
	freq_table->codes = calloc(MAX_NUM_BYTES, sizeof(char *));
	if (!freq_table->codes) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < freq_table->size; i++) {
		freq_table->freq[i] = 0;
		freq_table->codes[i] = NULL;
	}
	
	return freq_table;
}

/* Puts the frequencies of all characters in a file into a freq table */
void genFreq(int fdin, int size, FrequencyTable *freq_table) {
	int i;
	unsigned int c = 0;
	int status;
	/* read each character from file and put into frequency table */
	for (i = 0; i < size; i++) {
		/* Read a char into c */
		status = read(fdin, &c, sizeof(char));
		
		/* Check for error reading file */
		if (status == -1) {
			perror("error reading file");
			close(fdin);
			exit(EXIT_FAILURE);
		}
		if (freq_table->freq[c] == 0) {
			freq_table->unique_count += 1;
		}
		freq_table->freq[c] += 1;
		freq_table->count += 1;
	}
}

/* Free the frequency table */
void ftableDestroy(FrequencyTable *freq_table) {
	int i;
	for (i = 0; i < MAX_NUM_BYTES; i++) {
		if (freq_table->codes[i]) {
			/* Frees the codes */
			free(freq_table->codes[i]);
		}
	}
	/* Frees the code table */
	free(freq_table->codes);
	/* Frees the table */
	free(freq_table);
}

