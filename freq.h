#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef FREQH
#define FREQH

#define MAX_NUM_BYTES 256

/* Frequency Table contains a size which will be 256 to represents bits 0-255.
 * C */
typedef struct FrequencyTable {
	/* Total number of chars. in the file */
	unsigned int count;
	/* Number of unique chars. in the file. */
	unsigned int unique_count;
	/* Total size of the frequency table which is 256 */
	unsigned int size;
	/* An unsigned integer array of size 256. The index of the array is the 
	 * ASCII value of a character and the data at that index is the 
	 * frequency. */
	unsigned int freq[MAX_NUM_BYTES];
	/* Array of character pointers to hold each character's encoded code */
	char **codes;
	
} FrequencyTable;

FrequencyTable *makeFreqTable(void);
void genFreq(int, int, FrequencyTable *);
void ftableDestroy(FrequencyTable *);
#endif
