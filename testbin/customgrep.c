#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "divineregex.c"

#define CG_NMATCH 16

int main(int argc, char **argv)
{
	FILE *file=NULL;
	struct stat sb;
	char *line=NULL;
	regex_t *preg;
	size_t nmatch	= CG_NMATCH;
	regmatch_t *pmatch=malloc(sizeof(regmatch_t)*CG_NMATCH);
	size_t n = 0;
	int i;
	int j;
	int c;
	int eflags=0;
	int error=0;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <pathname> <regex>\n", argv[0]);
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
		preg=malloc(sizeof(regex_t));
		if(regcomp(preg,argv[2],REG_EXTENDED)==0)
		{
			while((getline(&line,&n,file))!=-1)
			{
				error=regexec(preg, line, nmatch, pmatch, 0);
				if (error == 0)
				{
					printf("%s",line);
					for(i=0;
					    i < CG_NMATCH;
					    i++)
					{
					printf("rm_so[%d]:%d	",i,(int)pmatch[i].rm_so);
					printf("rm_eo[%d]:%d	",i,(int)pmatch[i].rm_eo);
					putchar('\'');
					for(j=pmatch[i].rm_so;
					    j<pmatch[i].rm_eo;
					    j++)
					{
						putchar(line[j]);
					}
					putchar('\'');
					putchar('\n');
					}
				}
			}
			if(fclose(file)==0)
			{
				printf("file closed\n");
			}else{
				printf("error closing file\n");
			}
			free(line);
			regfree(preg);
			free(preg);
		}else{
			printf("no regcomp\n");
			perror(NULL);
		}
	}


	return 0;
}
