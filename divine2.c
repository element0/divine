#include <stdio.h>		// provides FILE,fopen, etc...
#include <stdlib.h>		// provides __secure_getenv(),char **environ
#include <string.h>		// provides strdup(), NOTE: see man strdup for glibc compat
extern char **environ;

//== TYPES: HYENA ==

typedef struct Fileobj
{
	char *uri;
	char *localpath;
	FILE *file;
	int retaincount;
}Fileobj;

typedef struct Datapoint
{
	struct Fileobj *file;
	long fpos;
}Datapoint;

typedef struct Scanner
{
	Datapoint 	*(*test_scanner_on_data)	(Datapoint *dpoint);
	Datapoint 	*(*find_start_datapoint)	(Datapoint *dpoint);
	Datapoint	*(*set_next_datapoint)		(Datapoint *dpoint);
	void 		*(*parse_start_matter)		(Datapoint *dpoint);
	void 		*(*parse_end_matter)		(Datapoint *dpoint);
	char		(*ascertain_relationship_to_node)	(Datapoint *dpoint);
	int		(*eof)				(Datapoint *dpoint);
	
}Scanner;


//== TYPE FUNCTIONS: HYENA ==
Scanner *new_scanner()
{
	Scanner *scanner=malloc(sizeof(Scanner));

	return scanner;
}

int rm_scanner(Scanner **scanner)
{
	if(*scanner != NULL)
	{
		free(*scanner);
		*scanner=NULL;
	}
	return 0;
}

int rm_datapoint(Datapoint *datapoint)
{
	if(datapoint != NULL)
	{
		if(datapoint->file != NULL)
		{
			if(datapoint->file->localpath != NULL)
			{
				free(datapoint->file->localpath);
			}
 			free(datapoint->file); 
		}
		free(datapoint);
	}
	return 0;
}

Datapoint *new_datapoint(char *url)
{
	Datapoint *datapoint=malloc(sizeof(Datapoint));
	if(datapoint != NULL)
	{
		datapoint->file=malloc(sizeof(Fileobj));
		if(datapoint->file == NULL)
		{
			rm_datapoint(datapoint);
			datapoint=NULL;
		}
	}
	return datapoint;
}





//== SCANNER: OX ==
// depends TYPES:HYENA
Datapoint	*ox_test_scanner_on_data(Datapoint *dpoint)	{	printf("1");	}
Datapoint	*ox_find_start_datapoint(Datapoint *dpoint)	{	printf("2");	}
Datapoint	*ox_set_next_datapoint	(Datapoint *dpoint)	{	printf("3");	}
void 		*ox_parse_start_matter	(Datapoint *dpoint)	{	printf("4");	}
void 		*ox_parse_end_matter	(Datapoint *dpoint)	{	printf("5");	}
char		 ox_ascertain_rel_to_node (Datapoint *dpoint)
{	
	static int count = 0;
	switch(count++)
	{
		case 0:
			return 'c';
		case 1:
			return 's';
		default:
			return 'o';
	}
}

int		 ox_eof			(Datapoint *dpoint)	{	return EOF;	}


Scanner *new_ox()
{
	Scanner *scanner=new_scanner();

	scanner->test_scanner_on_data	=	&ox_test_scanner_on_data;
	scanner->find_start_datapoint	=	&ox_find_start_datapoint;
	scanner->set_next_datapoint	=	&ox_set_next_datapoint;
	scanner->parse_start_matter	=	&ox_parse_start_matter;
	scanner->parse_end_matter	=	&ox_parse_end_matter;
	scanner->ascertain_relationship_to_node=&ox_ascertain_rel_to_node;
	scanner->eof			=	&ox_eof;

	return scanner;
}


//== MAIN: HYENA  ==
int parse_tree(Scanner *scanner, Datapoint *datapoint)
{
	static int 	total_depth	= 1;
	int 		current_depth	= total_depth++;
	if(scanner==NULL)
	{
		scanner = new_ox();
		scanner->find_start_datapoint(datapoint);
	}
	
	scanner->parse_start_matter(datapoint);
	scanner->set_next_datapoint(datapoint);
	//hyena_ascertain_scanner_for_next_node
	do
	{
		switch(scanner->ascertain_relationship_to_node(datapoint))
		{
			case 'c':		// child
				printf("child\n");
				parse_tree(scanner,datapoint);
				scanner->parse_end_matter(datapoint);
				break;
			case 's':		// sib
				printf("sib\n");
				scanner->parse_end_matter(datapoint);
				parse_tree(scanner,datapoint);
				break;
			case 'o':		// other
				printf("other\n");
				return 0;
				break;
		}

	}while(scanner->eof(datapoint) != EOF);

	
	if(current_depth==1) { rm_scanner(&scanner); }
	return 0;
}



/*---------- MAIN ------------*/
// usage: hyena [ -s scanner ] < file | URL >

int main(int argc, char **argv)
{
	/*----- TEST CORRECT NUM ARGS -----*/
	if(argc!=2)
	{
		printf("Usage: %s <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*----- SETUP DATA STRUCTURES -----*/
	Datapoint *datapoint = new_datapoint();
	if(datapoint == NULL)
	{
		printf("hyena error: datapoint == NULL\n");
		exit(EXIT_FAILURE);
	}

	/*----- SET INPUT -----*/
	datapoint->file->localpath = strdup(argv[1]);

	parse_tree(NULL,datapoint);

	/*----- FREE DATA STRUCTURES -----*/
	rm_datapoint(datapoint);
	
	return 0;
}
