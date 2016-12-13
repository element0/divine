#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	FILE *file=NULL;
	struct stat sb;
	char *lineptr;
	lineptr[0]='\0';
	size_t n = 0;
	int c;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if(stat(argv[1], &sb) == -1)
	{
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	if(ferror(file=fopen(argv[1],"r")))
	{
		perror(NULL);
		printf("error opening file\n");
		exit(EXIT_FAILURE);
	}else{
		printf("file opened\n");
		while((getline(&lineptr,&n,file))!=-1)
		{
			printf("%s",lineptr);
		}
		perror(NULL);
		if(fclose(file)==0)
		{
			printf("last lineptr=%s\n",lineptr);

			printf("file closed\n");
		}else{
			printf("error closing file\n");
		}
	}

	free(lineptr);

	return 0;
}
