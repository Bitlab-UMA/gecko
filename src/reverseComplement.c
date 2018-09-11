/* reverseComplement.c
   Program reading a FASTA file as input (one or multiple sequence)
   and then returning the reverse complementary sequence in the output file.

   It is important to note that for multiple sequence file the first input
   sequence is the last one at the output file and the last one in the input
   is the first one in the output.
   oscart@uma.es
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "commonFunctions.h"

#define SEQSIZE 2000000000
#define WSIZE 32
#define NREADS	5000000
#define BREAK_TIME 60

int main(int ac, char** av) {
	FILE *fIn, *fOut;
	int64_t i, j, nR, nReads = NREADS, seqLen = 0;
	char *seq, c, toW;
	long *offset;
	uint64_t time_to_put_break;

	if (ac != 3)
		terror("USE: reverseComplement seqFile.IN reverseComplementarySeq.OUT");

	/**
	 * Allocating memory for the sequence
	 * Fixed amount of memory, change it to loadSeqDB?
	 */
	if ((seq = (char*) malloc(SEQSIZE)) == NULL)
		terror("memory for Seq");

	if ((fIn = fopen(av[1], "rt")) == NULL)
		terror("opening IN sequence FASTA file");

	if ((fOut = fopen(av[2], "wt")) == NULL)
		terror("opening OUT sequence Words file");

	if ((offset = (long*) calloc(nReads, sizeof(long))) == NULL)
		terror("memory for offset vector for reads");

	nR = 0;
	c = fgetc(fIn);
	while(!feof(fIn)){
		if(c == '>'){
			if(nR >= nReads){
				nReads += NREADS;
				long *more_offset = NULL;
				more_offset = realloc(offset, nReads * sizeof(long));
				
				if(more_offset == NULL)
					terror("memory for offset vector for reads");

				offset = more_offset;
			}

			offset[nR++] = ftell(fIn)-1;
		}
		c = fgetc(fIn);
	}

	for(i=nR-1; i>=0; i--){
		fseek(fIn, offset[i], SEEK_SET);
		//READ and write header
		if(fgets(seq, SEQSIZE, fIn)==NULL){
			fprintf(stdout, "Error reading read: %" PRIi64 " with offset: %ld\n", i, offset[i]);
			terror("Empty file");
		}
		fprintf(fOut, "%s", seq);
		//READ and write sequence
		c = fgetc(fIn);
		while(c != '>' && !feof(fIn)){
			if(isupper(c) || islower(c)){
				seq[seqLen++]=c;
			}
			c = fgetc(fIn);
		}
		time_to_put_break = 0;
		for(j=seqLen-1; j >= 0; j--){
			switch(seq[j]){
				case 'A':
					toW = 'T';
					break;
				case 'C':
					toW = 'G';
					break;
				case 'G':
					toW = 'C';
					break;
				case 'T':
					toW = 'A';
					break;
				case 'U':
					toW = 'A';
					break;
				case 'a':
					toW = 't';
					break;
				case 'c':
					toW = 'g';
					break;
				case 'g':
					toW = 'c';
					break;
				case 't':
					toW = 'a';
					break;
				case 'u':
					toW = 'a';
					break;
				default:
					toW = seq[j];
					break;
			}
			fwrite(&toW, sizeof(char), 1, fOut);
			time_to_put_break++;
			if(time_to_put_break > BREAK_TIME){ toW = '\n'; fwrite(&toW, sizeof(char), 1, fOut); time_to_put_break = 0; }
		}
		toW='\n';
		fwrite(&toW, sizeof(char), 1, fOut);
		seqLen=0;
	}

	fclose(fIn);
	fclose(fOut);

	return 0;
}

