#include <math.h>
#include "cachesim.h"

int checkPowerof2(int inputX){ //check if input is a power of 2
    if(inputX == 0) return 1;
    while(inputX != 1){
        if(inputX%2 != 0) return 1;
        inputX = inputX/2;
    }
    return 0;
}

int solveForExponent(int B){ //Solves for the exponent of 2^x
    double y = (double)B;
    double a = log(y);
    double expono = a/(log(2));
    return ((int)expono);
}

int newTagOrIndex(long long int *newAddress, int num, int start){  //Calculates the new tag or index
    int counter = 0, valid = 1, temp = 0, end = num+start;
    while(start < end){
        if(newAddress[start] == valid){
            counter += pow(2, num - temp -valid);
        }
        start++;
        temp++;
    }
    return counter;
}

char work(cache **cMatrix, long long int Address, int tag, int index, int n_way, int isPreF, int bS, char* policy){
    long long int *newAdd, empty = -1;
    char in;
    int i, j, sizeOfAddress, filled = n_way -1;

    if(isPreF == 1) Address += bS;   //When working with prefetcher, add blockSize to the address

    //Allocates space for the newAddress
    newAdd = malloc(48 * sizeof(long long int));
    
    sizeOfAddress = 47;
    while(sizeOfAddress > 0){
        newAdd[sizeOfAddress] = Address % 2;
        Address = Address/2;
        sizeOfAddress = sizeOfAddress - 1;
    }

    int valT = newTagOrIndex(newAdd, tag, 0);

    int place = newTagOrIndex(newAdd, index, tag);

    int k;
    for(i = 0; i < n_way; i++){
        if(cMatrix[place][i].tStorage == empty){ //Cache block is empty -> miss
            cMatrix[place][i].tStorage = valT; //Storages tag
            cMatrix[place][i].rank = 1;   //Most recently used
            for (k = 0; k < n_way; k++){    //iterates through the ranks and rearanges them accordingly
                if(i != k && cMatrix[place][k].rank != 0){    //Doesnt change most recently used or empty ranks
                    cMatrix[place][k].rank += 1;
                }
            }
            in = 'n';
            break;
        }else if (cMatrix[place][i].tStorage == valT){ //Tag is found -> hit
            if(cMatrix[place][i].rank != 1){  //If rank = 1 no need to change ranks, else order the ranks
                cMatrix[place][i].rank = 1; //Since accessed most recently used
                for (k = 0; k < n_way; k++){
                    if(i != k && cMatrix[place][k].rank != 0){//Doesnt change most recently used or empty ranks
                        cMatrix[place][k].rank += 1;
                    }
                }
            }
            in = 'y';
            break;
        }

        if(i == filled){ //The last index has been reached -> use replacement policy
            /*LRU
             *Uses a rank system for lru
             *Rank 1 = most recently used
             *Lowest rank (biggest number) -> least recently used
             */
            if(strcmp(policy, "lru") == 0){
                int largest = cMatrix[place][0].rank, indexNumber = 0;
                for (j = 0; j < n_way; j++){    //Finds the least recently used tag by finding the lowest ranked and its position
                    if(largest < cMatrix[place][j].rank){
                        largest = cMatrix[place][j].rank;
                        indexNumber = j;
                    }
                }
                cMatrix[place][indexNumber].tStorage = valT;  //replaces it
                cMatrix[place][indexNumber].rank = 1; //Most recently used
                for (k = 0; k < n_way; k++){
                    if(indexNumber != k){
                        cMatrix[place][k].rank += 1;
                    }
                }
                //break;
            }else{
                /*FIFO -> Ignores the rank system*/
                for(j = 0; j < i; j++){
                    cMatrix[place][j] = cMatrix[place][j+1];
                }
                cMatrix[place][i].tStorage = valT;
            }
            in = 'n';
        }
    }
    free(newAdd);   //Free memory
    return in;
}

/*Frees the allocated cache*/
void freeCache(cache ** Cache, int rows){
    int i;
    cache *current;
    for(i = 0; i < rows; i++){
        current = Cache[i];
        free(current);
        current = NULL;
    }
    free(Cache);
    Cache = NULL;
}

int main(int argc, char **argv){
    if(argc != 6){
        printf("Argument parameters are Incorrect! Exiting program...\n");
        return 0;
    }

    /*Places arguments into variables*/
    int cacheSize = atoi(argv[1]);
    char* assoc = argv[2];
    char* policy = argv[3];
    int blockSize = atoi(argv[4]);
    FILE* fp = fopen(argv[5], "r");   

    if(fp == NULL){ //Checks to see if the file is empty
        printf("File is empty! Exiting program...\n");
        return 0;
    }

    /*Determines what kind of cache is being used*/
    int typeAssoc;
    int n;
    if(strcmp(assoc,"direct") == 0) typeAssoc = 1;  //Direct mapped cache
    else if(strcmp(assoc,"assoc") == 0) typeAssoc = 2;  //Fully associative cache
    else if(strncmp(assoc,"assoc:",6) == 0){ //n - way associative cache
        const char search = ':';
        char* ret = strchr(assoc,search);
        ret++; //Deletes ':' from the string
        n = atoi(ret);
        if(checkPowerof2(n) == 1){ //Check to see if n is not a power of 2
            printf("Invaild number for associativity, not a power of 2! Exiting...\n");
            return 0;
        }
        typeAssoc = 3;
    }

    if(strcmp(policy,"fifo") != 0 && strcmp(policy,"lru") != 0){    /*Checks if the policy is valid*/
        printf("Policy is not valid! Exiting program...\n");
        return 0;
    }

    /*Determines if blockSize is vaild*/
    if(checkPowerof2(blockSize) == 1 || blockSize > cacheSize){ //to see if block Size is not a power of 2
        printf("Invalid Block Size! Exiting...\n");
        return 0;
    }

    /*Determines Set and Associativity*/
    int numSets;
    int n_way;
    switch(typeAssoc){
        case 1: //direct
            n_way = 1;
            numSets = ((cacheSize)/(blockSize));
            break;
        case 2: //assoc
            n_way = ((cacheSize)/(blockSize));
            numSets = ((cacheSize)/(blockSize * n_way));
            break;
        case 3: //assoc:n
            numSets = ((cacheSize)/(blockSize * n));
            n_way = n;
            break;
    }

    int offSet = solveForExponent(blockSize);  //blockSize is equal to 2^x, offset = x
    int tag, index = 0;
    if(typeAssoc == 1 || typeAssoc == 3) index = solveForExponent(numSets); //index is equal to x of numSets = 2^x
    tag = 48 - offSet - index;
    
    /*Allocates space for both first and second cache -> 2d array*/
    int i,j;
    cache **first = (struct cache**) malloc(sizeof(struct cache*) * numSets);
    cache **second = (struct cache**) malloc(sizeof(struct cache*) * numSets);
    for(i = 0; i < numSets; i++){
        first[i] = (struct cache*) malloc(n_way * sizeof(struct cache));
        second[i] = (struct cache*) malloc(n_way * sizeof(struct cache));
    }

    for (i = 0; i < numSets; i++){
        for(j = 0; j < n_way; j++){
            first[i][j].tStorage = -1;
            first[i][j].rank = 0;
            second[i][j].tStorage = -1;
            second[i][j].rank = 0;
        }
    }

    /*Prints out all variables for debugging purposes*/
    /*printf("cacheSize ->   %d\n", cacheSize);
    printf("assoc     ->   %s\n", assoc);
    printf("policy    ->   %s\n", policy);
    printf("blockSize ->   %d\n", blockSize);
    printf("typeAssoc ->   %d\n", typeAssoc);  
    printf("numSets   ->   %d\n", numSets);
    printf("n_way     ->   %d\n", n_way);
    printf("offset    ->   %d\n", offSet);
    printf("tag       ->   %d\n", tag);
    printf("index     ->   %d\n", index);
    printf("\n");*/

    /*Reads the file*/
    char useless[32];   //Used to store the first string(considered useless for this assigment)
    char opSymbol;   //Stores operation symbol
    long long int address; //Stores the hexidecimal
    char const write = 'W';
    char const stop[] = "#eof";

    int nPH = 0;
    int nPM = 0;
    int nPR = 0;
    int nPW = 0;
    int pH = 0;
    int pM = 0;
    int pR = 0;
    int pW = 0;
    char iah; // is a hit
    //int counter = 0; //Debugging

    while(fscanf(fp, "%s %c %llx", useless, &opSymbol, &address) != EOF){
            if(strcmp(useless,stop) == 0)   break;
               
                if(opSymbol == write){
                    iah = work(first, address, tag, index, n_way, 0, blockSize, policy);
                    nPW++;
                    //printf("Write\n");
                    switch(iah){
                        case 'y':   //Is A Hit
                            //printf("Known hit\n");
                            nPH++;
                            break;
                        case 'n':   //Is A Miss
                            //printf("Known miss\n");
                            nPR++;
                            nPM++;
                            break;
                        default:    //Error
                            printf("Error: Incorrect input for var 'iah'\n");
                            break;
                    }
                    //break;
                }else{
                    iah = work(first, address, tag, index, n_way, 0, blockSize, policy);
                    switch(iah){   
                        case 'y':   //Is A Hit
                            //printf("Known hit\n");
                            nPH++;
                            break;
                        case 'n':   //Is A Miss
                            //printf("Known miss\n");
                            nPR++;
                            nPM++;
                            break;
                        default:    //Error
                            printf("Error: Incorrect input for var 'iah'\n");
                            break;
                    }
                }

                
                if(opSymbol == write){
                    //printf("binary -> %s\n", binary);
                    iah = work(second, address, tag, index, n_way, 0, blockSize, policy);
                    pW++;
                    //printf("Write\n");
            
                    switch(iah){
                        case 'y':   //Is A Hit
                            //printf("Known hit\n");
                            pH++;
                            break;
                            
                        case 'n':   //Is A Miss
                            //printf("Known miss\n");
                            pR++;
                            pM++;
                            break;
                        
                        default:    //Error
                            printf("Error: Incorrect input for var 'iah'\n");   //debugging
                    }
                }else{
                    //printf("binary -> %s\n", binary);
                    iah = work(second, address, tag , index, n_way, 0, blockSize, policy);
                    switch(iah){
                        case 'y':   //Is A Hit
                            //printf("Known hit\n");
                            pH++;
                            break;
                            
                        case 'n':   //Is A Miss
                            //printf("Known miss\n");
                            pR++;
                            pM++;
                            break;
                       
                        default:    //Error
                            printf("Error: Incorrect input for var 'iah'\n");
                    }
                }
                if(iah != 'y'){
                    //binaryTemp = malloc(strlen(bin) + 1);
                    //strcpy(binaryTemp, bin);
                    //printf("temp - > %s\n", binaryTemp);
                    //printf("temp  - > %s\n", binaryTemp);
                    //printf("||||||||||||||||||||||||||||||||||||||||||||||\n");
                    //printf("temp  - > %s\n", binaryTemp);
                    iah = work(second, address, tag, index, n_way, 1, blockSize, policy);
                    if(iah == 'n'){
                          pR++;
                    }
                }
        
            //free(binary);
            //free(binaryTemp);
            //printf("counter: = %d\n", counter);
            //if(counter == 3) break;
            //break;
    }

    //print results
    printf("no-prefetch\n");
    printf("Cache hits: %d\n", nPH);
    printf("Cache misses: %d\n", nPM);
    printf("Memory reads: %d\n", nPR);
    printf("Memory writes: %d\n", nPW);
    printf("with-prefetch\n");
    printf("Cache hits: %d\n", pH);
    printf("Cache misses: %d\n", pM);
    printf("Memory reads: %d\n", pR);
    printf("Memory writes: %d\n", pW);

    //Free memory
    freeCache(first, numSets);
    freeCache(second, numSets);
    return 0;
}