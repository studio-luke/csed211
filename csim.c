#include "cachelab.h"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int s;
	int E;
	int b;

	int S_siz;
	int B_siz;
} cache_conf;

void print_help(char *argv[]) { 
	printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print usage\n");
    printf("  -v         Turn on the verbose mode.\n");
	printf("			 Print every trace with hit/miss/evition.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    exit(0);
}


int main(int argc, char *argv[])
{
	/* Main features for running cache simulator  */
	cache_conf cache_prop = { 0, 0, 0 };	// stores s, E, b, and extra constants
	FILE *trace;			// stores trace file stream
	int is_verbose = 0;		// verbose mode

	int **cache;
	char instr[20];
	int i;

	/* Temporary variables for user input */
	char trace_name[32];	// filename of trace input
	int opt;
	
	while ((opt = getopt(argc, argv, "s:E:b:t:hv")) != -1) { 
		switch(opt) {
		case 's': cache_prop.s = atoi(optarg); break;
		case 'E': cache_prop.E = atoi(optarg); break;
		case 'b': cache_prop.b = atoi(optarg); break;
		case 't': strcpy(trace_name, optarg); break;
		case 'h': print_help(argv); return 0;
		case 'v': is_verbose = 1; break;
		default:
			printf("\nError: Option '%c' doesn't exists.\n", opt);
			printf("To check the usage, type option -h\n\n");
			return 10;
		}
	}
	if (!cache_prop.s || !cache_prop.E || !cache_prop.b) {
		printf("\nError: Missing some necessary parameters...\n");
		printf("To check the usage, type option -h\n\n");
		return 11;
	}

	trace = fopen(trace_name, "r");
	if (trace == NULL) {
		printf("\nError: Invalid file name '%s'...\n", trace_name);
		printf("Cache Simulator Terminates.\n\n");
		return 12;
	}
	
	cache_prop.S_siz = 1 << cache_prop.s;
	cache_prop.B_siz = 1 << cache_prop.b;
	cache = (int**)calloc(cache_prop.S_siz, sizeof(int*));
	for (i = 0; i < cache_prop.S_siz; i++)
		cache[i] = (int*)calloc(cache_prop.B_siz, sizeof(int));
	printf("%d\n", cache[0][0]);

	while (fgets(instr, sizeof(instr), trace) != NULL) {
		if (instr[0] == 'I') continue;
		
		switch (*strtok(instr, " ,")) {
		case 'L':
		case 'S':
		case 'M':
			
		defalut: printf("ERROR!!"); exit(-1);
		}

	}

	if (!is_verbose)
		printSummary(0, 0, 0);
	
	for (i = 0; i < cache_prop.S_siz; i++)
		free(cache[i]);
	free(cache);
	fclose(trace);
	return 0;
}
