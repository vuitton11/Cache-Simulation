#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Creates a struct for cache
typedef struct cache{
	long long int rank;  //Keeps track of the rank used for lru policy
    long long int tStorage; //Stores the tags
}cache;

//Function Call
int checkPowerof2(int x);

int solveForExponent(int B);

int newTagOrIndex(long long int *newAddress, int num, int start);

char work(cache** cMatrix, long long int Address, int tag, int index, int n_way, int isPreF, int bS, char* policy);

void freeCache(cache ** Cache, int nos);