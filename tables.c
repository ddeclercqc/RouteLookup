#include "tables.h"

void create_table(infTable* forw_table){											//Initialisation of the elements of the tables
forw_table->first_table = (unsigned short*)calloc(NUMBER_ENTRIES, sizeof(short));  		//note: NUMBER_ENTRIES is defined inside tables.h as a constant (2²⁴ entries as we need them for the main table). We initialize the first table
 forw_table->second_table= (unsigned short*)calloc(256, sizeof(short));		      		//Initialize the second table with the corresponding 256 entries needed (we will expand it later if needed)
forw_table->N = 0; 													     		//Initialization of the variable used in order to expand the table later on if needed (as we will need more than one second table, this will help us to distinguish them)
}

//function that will insert an output interface for each existing prefix

void insert_prefix(infTable* forw_table, uint32_t prefix,int prefix_length,int output_interface, int *default_interface){

	if(prefix_length==0){									//Case in which we can't find any output interface, we assign a default one to avoid possible MISS (when the netmask is 0)
		*default_interface=output_interface;
	}
	if(prefix_length <=24) {									//The mask of the IP address is less than 24 therefore we stay in the main table and fill only this one
		int first_pos = prefix>>8;							//Get the 24 first bits associated with the first position to fill
		int last_pos = first_pos + pow(2, (24-prefix_length));		//For the last position, we need to fill it up depending on the length of the prefix (in this case we do 24 - prefix_length as this length will always be smaller or equal than 24)
		for(int j=first_pos;j<last_pos;j++){
			forw_table->first_table[j]=output_interface;		//assign output interfaces for all positions in the first table from first_pos to last_pos
		}
	}
	else{																											//The mask of the IP address is higher than 24 so we will need to go to the second table;

		if((forw_table->first_table[prefix>>8] & FIRST_OUTPUT_BIT)==0){														//the first bit of the output interface is 0 so we need to create the auxiliary table

			forw_table->second_table = (unsigned short*) realloc(forw_table->second_table,((forw_table->N+1)*256)*sizeof(short));	//we need this line to expand the second table that will increase with size N
			for(int j=255&prefix; j<((255&prefix)+pow(2, (32-prefix_length)));j++){												//same loop as previously but this time the prefixes are bigger than 24. With 255&prefix, we keep the last 8 bits as we need to assign a position to them
				forw_table->second_table[forw_table->N*256+j] = (short)output_interface;										//assign output interfaces for all positions in the second table 	
			}			

			forw_table->first_table[prefix>>8] = FIRST_OUTPUT_BIT | forw_table->N;											//we put a 1 in the first bit of the output_interface of the main table, which means that a second table has been created for the corresponding address
			forw_table->N +=1;																						//update the variable to expand the second table
		}		
		else{																										//the first bit of the output interface is 1 so a second table has already been created (no need to do a realloc this time)
			int pos=forw_table->first_table[prefix>>8] & 0x7FFF;															//Here, we get the position to fill the second table
			for(int j=255&prefix; j<((255&prefix)+pow(2, (32-prefix_length)));j++){												//same loop as previously but this time the prefixes are bigger than 24. With 255&prefix, we keep the last 8 bits as we need to assign a position to them
				forw_table->second_table[pos*256+j] = (short)output_interface;												//assign output interfaces for all positions in the second table (we do pos*256 because we need to choose among different second tables, all of them with 256 positions, but not with the same first position, which increase by 256 every time
			}

		}
	
	}
}

//Function in charge of searching inside the tables given the prefix and which gives us the output interface for each of them

void lookup_search(infTable* forw_table, uint32_t prefix_lookup, int *output_interface_lookup, int *number_accesses, int default_interface){

	*output_interface_lookup = forw_table->first_table[prefix_lookup>>8];

	if(*output_interface_lookup==0){																//same as previously, we need a default interface to cover the MISS cases
		*output_interface_lookup = default_interface;
	}

	if((*output_interface_lookup & FIRST_OUTPUT_BIT)==0){											//the first bit of the output interface is 0 so we stay in the main table 
		*number_accesses=1;																		//only 1 table access as we stay in the main table
		return;
	}

	else{																						//the first bit of the output interface is 1 so we need to go the second table
		*number_accesses=2;																		//2 accesses this time as we access the first table as well as the second one
		int numtabla = *output_interface_lookup & 0x7FFF;												//Here, we get the position to get the output interface from the second table										
		*output_interface_lookup = forw_table->second_table[numtabla*256 + (prefix_lookup & 0xFF)];		//get the output interface after searching in the second table ( thanks to the previously calculated position and the input prefix (prefix_lookup) )
		return;
	}
	return;
}


void liberate_memory_tables(infTable* forw_table){													 //free the memory we have used previously by creating the tables
	if(forw_table){
		free(forw_table->first_table);
		free(forw_table->second_table);
	}
}
