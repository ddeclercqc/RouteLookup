#include "main.h"
#include "utils.h"
#include "io.h"
#include "tables.h"


int main(int argc, char **argv){

//Check correct number of arguments and exit if there is a number of arguments different than 3

if(argc != 3){
	printf("The program must have 3 input arguments");
	return -1;
}

//Initialize needed files and manage the errors (function printIoExplanationError will print the different errors on each case)
 
int init= initializeIO(argv[1],argv[2]);  //function that allows us to use the routing table and a file containing the addresses to be processed and print an error if we can't find anything
if(init!=0){
	printf("The following error has occured : ");
	printIOExplanationError(init);
	return -1;
}

//Table creation and inizitalization
struct forw_table table;
create_table(&table);

//Variables used in the functions provided as well as the created ones (inside tables.c). They are related to the insertion of the output interface thanks to the prefix (function insert_prefix in tables.c)
int output_interface;
uint32_t prefix;
int prefix_length;

//Variables used in the functions provided as well as the created ones (inside tables.c). They are related to lookup (function lookup_search in tables.c)
uint32_t prefix_lookup;
int output_interface_lookup=0;

//Variable that stores the number of accessed tables and the default interface that we will need to cover the cases in which we can't find any output interface (avoids MISS)
int number_accesses=0;
int default_interface=0;


//Variables and structures related to time calculations
struct timespec initialTime;
struct timespec finalTime;
double searchingTime;

//Variables related to the printed summary (thanks to the function inside io.c : printSummary)
int processedPackets=0;
double averageTableAccesses=0;
double averagePacketProcessingTime=0;
double totalTableAccesses=0;
double totalPacketProcessingTime=0;


//With this following function we will read the parameters of each line in the FIB file until it ends. 

while(readFIBLine(&prefix,&prefix_length,&output_interface)==OK){
	insert_prefix(&table,prefix,prefix_length,output_interface, &default_interface);  //function that will be in charge of filling the table. It will insert the output interface in the first or second table (depending on the netmask).
}

//With this following function we will read the parameters of each line in the InputPacketFile file until it ends. 

while(readInputPacketFileLine(&prefix_lookup)==OK){
	clock_gettime(CLOCK_MONOTONIC_RAW,&initialTime);  											//get initial time before searching in the tables 
	lookup_search(&table, prefix_lookup, &output_interface_lookup, &number_accesses, default_interface);	//function that will search inside the tables and will give us the output interface related to each input prefix
	clock_gettime(CLOCK_MONOTONIC_RAW,&finalTime);												//get final time (after the search finishes)
	printOutputLine(prefix_lookup, output_interface_lookup, &initialTime, &finalTime, &searchingTime, number_accesses); 	//it allows us to print a line in the desired format (address, output interface, number of accesed tables and time needed)
	processedPackets++;			
	totalTableAccesses += number_accesses;
	totalPacketProcessingTime += searchingTime;
	
}

//Calculation of the average number of accessed tables and the average processing time of the packets.
averageTableAccesses = totalTableAccesses/processedPackets;
averagePacketProcessingTime = totalPacketProcessingTime/processedPackets;

printSummary(processedPackets, averageTableAccesses, averagePacketProcessingTime);  //function which will print the necessary information such as the used memory, the average accesed tables, the CPU time, etc... given in io.c


//Memory freeing
freeIO();
liberate_memory_tables(&table);

return 0;

}