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

typedef struct {
	int valid;
	long long unsigned int tag;
	long timestamp;
} cache_line;

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
}


int main(int argc, char *argv[])
{
	/* Main features for running cache simulator  */
	cache_conf cache_prop = { 0, 0, 0 };	// stores s, E, b, and extra constants
	FILE *trace;			// stores trace file stream
	int is_verbose = 0;		// verbose mode

	cache_line **cache;
	char instr[20], buf[20];

	long long unsigned int addr, tag, dist;
	int block, set;

	int debug = 0;

	int hit = 0, miss = 0, eviction = 0;
	int is_hit, is_eviction, is_Mhit;
	int i, cnt = 0, min, min_idx;

	/* Temporary variables for user input */
	char trace_name[32];	// filename of trace input
	int opt;
	
	while ((opt = getopt(argc, argv, "s:E:b:t:hvd")) != -1) { 
		switch(opt) {
		case 's': cache_prop.s = atoi(optarg); break;
		case 'E': cache_prop.E = atoi(optarg); break;
		case 'b': cache_prop.b = atoi(optarg); break;
		case 't': strcpy(trace_name, optarg); break;
		case 'h': print_help(argv); return 0;
		case 'v': is_verbose = 1; break;
		case 'd': debug = 1; break;
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
	cache = (cache_line**)calloc(cache_prop.S_siz, sizeof(cache_line*));
	for (i = 0; i < cache_prop.S_siz; i++)
		cache[i] = (cache_line*)calloc(cache_prop.E, sizeof(cache_line));

	while (fgets(instr, sizeof(instr), trace) != NULL) {
		cnt++;
		if (instr[0] == 'I') continue;
		is_hit = is_Mhit = is_eviction = 0;
		instr[strcspn(instr, "\n")] = '\0';
		
		strcpy(buf, instr);
		switch (*strtok(buf, " ,")) {
		case 'M':
			is_Mhit = 1;
			hit += 1;
		case 'L':
		case 'S':
			addr = dist = strtol(strtok(NULL, " ,"), NULL, 16);
			
			block = dist % cache_prop.B_siz;
			dist /= cache_prop.B_siz;
			set = dist % cache_prop.S_siz;
			dist /= cache_prop.S_siz;
			tag = dist;

			if (debug){
				printf("┌-> [Timestamp %d] b: %d, s: %d, t: 0x%llx\n", cnt, block, set, tag);
				/*if (addr == 0x7ff000370 || 
					(block == 0 && set == 2 && (tag == 0x30054 || tag == 0x30051)))
					for (i = 0; i < cache_prop.E; i++)
						printf("├-> CACHE %d> v:%d, tag:0x%llx, time:%ld\n",
							i, cache[set][i].valid, cache[set][i].tag,
							cache[set][i].timestamp);*/
			}

			/* To be 'HIT', Search for the given data */
			for (i = 0; i < cache_prop.E; i++)
				if (cache[set][i].valid == 1 && cache[set][i].tag == tag) {
					cache[set][i].timestamp = cnt;
					
					hit += 1;
					is_hit = 1;
					break;
				}
			if (is_hit) break;
			miss += 1;

			is_eviction = 1;
			/* To be just 'MISS', Search for available space*/
			for (i = 0; i < cache_prop.E; i++)
				if (cache[set][i].valid == 0) {
					cache[set][i].valid = 1;
					cache[set][i].tag = tag;
					cache[set][i].timestamp = cnt;
					
					is_eviction = 0;
					break;
				}
			if (!is_eviction) break;
			
			/* To be 'EVICTION', LRU Policy*/
			eviction += 1;
			min_idx = 0;
			min = cache[set][min_idx].timestamp;
			for (i = 0; i < cache_prop.E; i++) {
				if (min > cache[set][i].timestamp) {
					min = cache[set][i].timestamp;
					min_idx = i;
				}
			}
			
			/*if (debug && block==0 && set==2 && tag==0x30051) 
				printf("Replacing cahce %d to tag 0x%llx(t=%d)\n", min_idx, tag, min);*/
			
			cache[set][min_idx].valid = 1;
			cache[set][min_idx].tag = tag;
			cache[set][min_idx].timestamp = cnt;
			break;
		default: printf("ERROR!!\n"); exit(-1);
		}
		if (is_verbose)
			printf("%s %s%s%s\n", instr + 1, is_hit ? "hit " : "miss ", 
										is_eviction ? "eviction " : "",
										is_Mhit? "hit " : "");
	}

	printSummary(hit, miss, eviction);
	
	for (i = 0; i < cache_prop.S_siz; i++)
		free(cache[i]);
	free(cache);
	fclose(trace);
	return 0;
}
