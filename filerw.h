#include <stdlib.h>
#include <unistd.h>

#ifndef FINFOH
#define FINFOH
#include "freq.h"
#include "llist.h"

void makeHeader(int, FrequencyTable *);
void makeBody(int, int, int, FrequencyTable *);
void decode(int, int, Node *, FrequencyTable *);
#endif

