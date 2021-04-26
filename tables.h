#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define NUMBER_ENTRIES 16777216 		//as 2²⁴ = 16777216 which corresponds to the number of entries of the first table
#define FIRST_OUTPUT_BIT 0x8000	       //constant used to check if the first bit of the output interface in the first table is 0 or 1

//We use a struct to declare our tables. It contains both tables and the variable that will allow us to expand the second table 

typedef struct forw_table {
unsigned short* first_table;
unsigned short* second_table;
int N;
}infTable;


//Definition of all the previously created functions in tables.c

void create_table(infTable* forw_table);
void insert_prefix(infTable* forw_table, uint32_t prefix,int prefix_length,int output_interface, int *default_interface);
void lookup_search(infTable* forw_table, uint32_t prefix_lookup, int *output_interface_lookup, int *number_tables, int default_interface);
void liberate_memory_tables(infTable* forw_table);