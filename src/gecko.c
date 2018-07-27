#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <float.h>
#include "structs.h"
#include "commonFunctions.h"
#include "dictionaryFunctions.h"
#include "comparisonFunctions.h"

char final_strand = 'f';

int main(int ac, char **av) {
    if (ac < 8) {
        terror("USE: gecko seqFileX.IN seqFileX.IN fragsFile.OUT Lmin SimTh WL strand *min_e_value");
    }

    char *seqX = av[1];
    char *seqY = av[2];
    char *out = av[3];

    uint64_t Lmin = (uint64_t) atoi(av[4]);
    uint64_t SimTh = (uint64_t) atoi(av[5]);
    int wSize = atoi(av[6]);
    final_strand = av[7][0];

    long double e_value = LDBL_MAX; //If no expected value was provided, use Long Double Max which will never happend
    if(ac == 9) e_value = (long double) atof(av[8]);

    uint64_t nEntriesX = 0;
    uint64_t seqXLen = 0;
    hashentryF *entriesX;
    uint64_t nEntriesY = 0;
    uint64_t seqYLen = 0;
    hashentryR *entriesY;

    uint64_t nFrags;

    pthread_t thX, thY;
    DictionaryArgs argsX, argsY;

    argsX.seqFile = strdup(seqX);
    argsX.nEntries = &nEntriesX;
    argsX.seqLen = &seqXLen;
    pthread_create(&thX, NULL, dictionary, (void *) (&argsX));

    argsY.seqFile = strdup(seqY);
    argsY.nEntries = &nEntriesY;
    argsY.seqLen = &seqYLen;
    if(final_strand == 'f'){
        pthread_create(&thY, NULL, dictionaryWithReverseButItsActuallyJustForward, (void *) (&argsY));
    }else if(final_strand == 'r'){
        pthread_create(&thY, NULL, dictionaryOnlyReverse, (void *) (&argsY));
    }else{
        terror("Wrong strand.");
    }
    

    //Wait:
    pthread_join(thX, (void **) &entriesX);
    pthread_join(thY, (void **) &entriesY);


    free(argsX.seqFile);
    free(argsY.seqFile);


    //Hits, SortHits and filterHits
    hitsAndFrags(seqX, seqY, out, seqXLen, seqYLen, entriesX, nEntriesX, entriesY, nEntriesY, wSize, Lmin, SimTh,
                 &nFrags, argsX.seqStats, argsY.seqStats, e_value);
    uint64_t i;

    for (i = 0; i < nEntriesX; i++) {
        free(entriesX[i].locs);
    }
    free(entriesX);
    for (i = 0; i < nEntriesY; i++) {
        free(entriesY[i].locs);
    }
    free(entriesY);

    free(argsY.seqStats);

    return 0;
}
