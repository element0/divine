/*--- divinity ---*/
/*
   Copyright (c)2014-2016 Raygan Henley
 */



/*--- DESCRIPTION: ---*/
/*   This program takes a text file as input			*/
/*   assumes there is a simple outline				*/
/*   rooted at the leftmost head of line			*/
/*   each line until blank line or newline			*/
/*   is assumed to be an item in the outline			*/
/*   Translates the outline into HTML.				*/




/*--- DONE / SHOULD WORK: ---*/
/*   Done: Test if file exists on launch.			*/
/*   Done: Non-matching macros are ignored.			*/



/*--- TEST: ---*/
/*   test: can macro expansion handle multiple words?		*/
/*   test: what happens when you try to open a binary file?	*/
/*		or xml or html or other format...		*/



/*--- TODO: / IMPROVE / ERRORS: ---*/

/*	HTTP HEADERS						*/
/*   improve: context and content appropriate headers		*/

/*	SYNTAX MODULES						*/
/*   improve: load pre-compiled regex from file on launch	*/
/*   improve: add modules for other markdown			*/

/*	DOCUMENTATION						*/
/*   improve: documentation: manpage				*/

/*	PACKAGING AND DISTRO					*/
/*   improve: distribution: dpkg				*/

/*	TESTING							*/
/*   improve: develop test generator				*/

/*	ERROR HANDLING						*/
/*   improve: internal error checking and handling		*/
/*   todo:    error logging					*/

/*	CONFIG SYSTEM						*/
/*   improve: change compile time options to CLI options	*/
/*   improve: external config file.				*/

/*	CUSTOMER SERVICE					*/



/*	RELEASE 1						*/
/*   docs:   comments						*/
/*   docs:   website						*/
/*   docs:   README						*/
/*   docs:   style-guide					*/

/*   errs:   add error checking where appropriate		*/

/*   help:   'bugbomb' email					*/
/*   help:   bug tracker					*/

/*   dist:   git repository					*/
/*   dist:   PGP						*/
/*   dist:   product verification key				*/

/*   todo:   handle HTML close tags in input source		*/
/*   todo:   force children of plain text to become siblings	*/
/*		or create div objects out of text nodes?	*/

/*   todo:   MACROS:						*/
/*   todo:   MACRO INSERT POINTS				*/
/*   todo:   external file					*/
/*   fix:     /$"/ inserts a lonely </div>			*/
/*   fix:     /"<pre>/ doesn't behave as /<pre>/		*/

/*	RELEASE 2						*/
/*   todo:   $multi $macro $expansion				*/
/*   todo:   $macro_iteration{n}				*/

/*   todo:   SHELL STYLE VARS:					*/

/*   todo:   CGI CONTENT SERVER:				*/
/*   todo:   read based on URI request				*/
/*   todo:   write post data from client			*/

/*	RELEASE 3						*/
/*   todo:   LEX SWITCHING					*/



/*----- INCLUDES -----*/
/* #define _GNU_SOURCE	*/	/* required by __secure_getenv() */

#include <stdio.h>
#include <stdlib.h>		/* provides __secure_getenv(), provides char **environ */
extern char **environ;
#include <stdarg.h>
#include <sys/types.h>		/* required by regex */
#include <sys/stat.h>		/* required by 'stat()' file test function */
#include <string.h>
#include <regex.h>
#include <errno.h>

/* next contains the regular expressions 	*/
/* which are used to match each type of line	*/
/* from the input file 				*/

#include "divinelex.c"


/*---------- COMPILE TIME OPTIONS -------------*/


#define IDENTS_IN_CLOSETAGS	0

/* IDENTS_IN_CLOSETAGS 1 outputs id="identname" in the close tags 		*/
/* in large bodies of nested HTML 'div's it makes it easier to read 		*/
/* this is an experimental fuck with HTML and may not work in all browsers? 	*/

#define OUTPUT_HTTP_HEADER	1

#define TABWIDTH 		8

/* TABWIDTH is how many spaces to count per tab of input 				*/
/* it is recommended NOT to mix spaces and tabs in the source file indents 		*/
/* but if you do, the program needs to know how many spaces to count for each tab. 	*/




/*----- OBJECT: Lex  -----*/
/* requires typedef struct Env							*/
/*	error: Lex and Env require each other 		  			*/

/* the following will be initialized to hold the compiled regex for lexing 	*/

typedef struct Lex
{
	regex_t *blankline_preg		;
	regex_t *markup_preg		;
	regex_t *html_noclose_preg	;
	regex_t *ident_preg		;
	regex_t *ident_preg2		;
	regex_t *text_preg		;
	regex_t *macro_preg		;
	regex_t *macro_label_preg	;
}Lex;

void rm_lex_item(regex_t *preg)
{
	if(preg!=NULL)
	{
		regfree(preg);	
		free(preg);
	}
	
}

void rm_lex(Lex *lex)
{
	rm_lex_item(lex->blankline_preg);
	rm_lex_item(lex->markup_preg);
	rm_lex_item(lex->html_noclose_preg);
	rm_lex_item(lex->ident_preg);
	rm_lex_item(lex->ident_preg2);
	rm_lex_item(lex->text_preg);
	rm_lex_item(lex->macro_preg);
	rm_lex_item(lex->macro_label_preg);

	free(lex);
}

void init_lex_item(regex_t **preg,const char *re)
{
	*preg=malloc(sizeof(regex_t));
	if(regcomp(*preg,re,REG_EXTENDED!=0))
	{
		printf("init_lex_error\n");
		free(*preg);
		*preg=NULL;
	}
}

/* void init_lex(Env *env) */
void init_lex(Lex **lex)
{
	// printf("&lex:	     %u	lex:        %u\n",&lex,lex);
	*lex=malloc(sizeof(Lex));
	// printf("&lex:	     %u	lex:        %u	*lex:  %u\n",&lex,lex,*lex);
	Lex *newlex=*lex;
	// printf("&newlex->blankline_preg:%u\n",&newlex->blankline_preg);

	init_lex_item(&newlex->blankline_preg,    DIVINE_BLANKLINE_RE);
	init_lex_item(&newlex->markup_preg,	    DIVINE_MARKUP_RE);
	init_lex_item(&newlex->html_noclose_preg, DIVINE_HTML_NOCLOSE_RE);
	init_lex_item(&newlex->ident_preg,	    DIVINE_IDENT_RE);
	init_lex_item(&newlex->ident_preg2,	    DIVINE_IDENT_RE2);
	init_lex_item(&newlex->text_preg,	    DIVINE_TEXT_RE);
	init_lex_item(&newlex->macro_preg,	    DIVINE_MACRO_RE);
	init_lex_item(&newlex->macro_label_preg,  DIVINE_MACRO_LABEL_RE);
}


/*----- OBJECT:  Env -----*/

/* the Env holds the global variables that are used throughout  		*/
/* MAR14 rm  unused 'pwd' var 							*/
/* MAR14 add output_env to track output indent					*/

typedef struct Env
{
	/* these are used per Node 						*/
	/* there can be more than one env if there are external macros 		*/
	/* each node references one env 					*/
	/* each env can be referenced by multiple nodes 			*/
	/* one output_env for the entire program referenced by multiple env's 	*/

	char *filepath			;

	FILE	*file			;
	int	retaincount		;
	struct  Env *output_env		;
	int	htmlpreflag		;

	/* these are used by the output_env and are global to the program 	*/
	int	outputindent		;
	struct	Lex *lex		;
}Env;

Env *new_env()
{	
	Env *env 	 = malloc(sizeof(Env));

	env->filepath	 = NULL;
	env->file	 = NULL;
	env->retaincount = 0;
	env->output_env  = NULL;
	env->htmlpreflag = 0;

	env->outputindent= 0;
	env->lex	 = NULL;

	return env;
}

int  rm_env(Env *envtodestroy)
{
	if(envtodestroy->lex!=NULL)
	{
		rm_lex(envtodestroy->lex);
	}

	free(envtodestroy);
	return 0;
}




/*----- OBJECT:  Node -----*/

/* this is the main structure that holds the parsed line of input 	*/
/* I was going to build an entire linked tree in memory while parsing 	*/
/* and then output the linked tree in a second step, but currently  	*/
/* that has not been necessary.  We can just use the Node struct to  	*/
/* hold temporary parse data per line until the line is output.  	*/

typedef struct Node
{
	/* set by main() */
	Env  *env;
	
	/* set by parse_node() */
	char *rawline			;
	char *string			;
	char *content			;
	int  type			;
	char *htmlelement		;
	char *tagid			;
	char *tagclass			;
	char *opentag			;
	char *closetag			;
	int  rawindent			;

	/* set by parse_tree() */
	int  indent			;
	int  macroflag			;
	int  macrostat			;
}Node;

Node *new_node(Env *env)
{
	Node *newnode=malloc(sizeof(Node));	
	
	newnode->rawline	=NULL;
	newnode->string		=NULL;
	newnode->opentag	=NULL;
	newnode->closetag	=NULL;
	newnode->type		=0;
	newnode->htmlelement	=NULL;
	newnode->tagid		=NULL;
	newnode->tagclass	=NULL;

	newnode->rawindent	=0;
	newnode->indent		=0;
	newnode->macroflag	=0;
	newnode->macrostat	=0;

	newnode->env		=env;
	newnode->env->retaincount++;

	return newnode;
}

int  rm_node(Node *nodetodestroy)
{
	if(nodetodestroy!=NULL)
	{
		if(nodetodestroy->rawline   != NULL) free(nodetodestroy->rawline); 
		if(nodetodestroy->string    != NULL) free(nodetodestroy->string); 
		if(nodetodestroy->opentag   != NULL) free(nodetodestroy->opentag);
		if(nodetodestroy->closetag  != NULL) free(nodetodestroy->closetag);
		if(nodetodestroy->htmlelement  != NULL) free(nodetodestroy->htmlelement);
		if(nodetodestroy->tagid     != NULL) free(nodetodestroy->tagid);
		if(nodetodestroy->tagclass  != NULL) free(nodetodestroy->tagclass);

		
		if(nodetodestroy->env->retaincount > 1)
		{
			nodetodestroy->env->retaincount--;
		}else{
			rm_env(nodetodestroy->env);
		}

		free(nodetodestroy);
	}
	
	return 0;
}


/*----------- PROTOTYPES ---------------*/

/*	This has to be declared early	*/

long setpos_at_macro(Node*);




/*----------- OUTPUT FUNCTIONS -----------*/

void output_indent(int outputindent)
{
	int i=0;
	for(i=0;i<outputindent;i++)
	{
		putchar('	');
	}
}

void output_opentag(Node *node)
{
	if(node->macroflag==0)
	{
		if(node->type==DIVINE_MACRO)
		{
			output_indent(node->env->outputindent);
			printf("MACRO_START\n");
		}
		if(node->type==DIVINE_TEXT)
		{
			/* handle <pre> formatted text: no added indent */
			if(node->env->htmlpreflag<1)
			{
				output_indent(node->env->outputindent);
				printf("%s\n",node->opentag);
			}else{
				printf("%s\n",node->opentag);
			}
		}

		if(node->type==DIVINE_IDENT   ||
		   node->type==DIVINE_CLASS)
		{
			output_indent(node->env->outputindent);
			printf("<%s",node->htmlelement);
			if((node->tagid)   !=NULL)  printf(" id=\"%s\"",node->tagid);
			if((node->tagclass)!=NULL)  printf(" class=\"%s\"",node->tagclass);
			printf(">\n");

		}

		if(node->type==DIVINE_MARKUP  ||
		   node->type==DIVINE_HTML_NOCLOSE )
		{
			if(node->opentag!=NULL)
			{
				output_indent(node->env->outputindent);
				printf("%s\n",node->opentag);
			}else{
				printf("error:node->opentag NULL");
			}
		}
		/* set htmlpreflag */
		if(node->htmlelement   !=NULL&&
		   node->htmlelement[0]=='p' &&
		   node->htmlelement[1]=='r' &&
		   node->htmlelement[2]=='e' &&
		   node->htmlelement[3]=='\0')
		{
			node->env->htmlpreflag+=1;
		}
	}else{
		/* macroflag != 0 */
		/* output nothing */
	}
}

void output_closetag(Node *node)
{
	if(node->macroflag==0)
	{
		if( node->type==DIVINE_TEXT         ||
		    node->type==DIVINE_MACRO        ||
		    node->type==DIVINE_HTML_NOCLOSE ||
		    node->macrostat==-1)
		{
			return;
			/* do not output closetag	*/
		}

		if(node->htmlelement!=NULL)
		{
			if( (node->tagid!=NULL) &&
			    (IDENTS_IN_CLOSETAGS==1) )
			{
				output_indent(node->env->outputindent);
				printf("</%s id=\"%s\">\n",node->htmlelement,node->tagid);
			}else{
				output_indent(node->env->outputindent);
				printf("</%s>\n",node->htmlelement);
			}
			/* close htmlpreflag */

			if( (node->htmlelement[0]=='p') &&
			    (node->htmlelement[1]=='r') &&
			    (node->htmlelement[2]=='e') &&
			    (node->htmlelement[3]=='\0'))
			{
				node->env->htmlpreflag+=-1;
			}
		}
	}else{
		/* macroflag != 0 */
		/* print nothing */
	}
}



/*------------ FILE SEARCH FUNCTIONS ------*/
long setpos_at_macro(Node *node)
{
	char 	*line		= NULL;
	size_t	linelen		= 0;
	regex_t preg		;
	size_t  nmatch		= 1;
	regmatch_t *pmatch	= malloc(sizeof(regmatch_t)*nmatch);
	char	*strncpyerr	= NULL;
	int	reerror		;
	int     returnval	= 0;
	long	lastfp		;
	int	searchstringlen = strlen(node->opentag)+7;
	char    *searchstring	= malloc(sizeof(char)*searchstringlen);

	searchstring[0] = '^';
	strncpyerr	= strncpy(searchstring+1,node->opentag,strlen(node->opentag));
	searchstring[searchstringlen-6]=':';
	searchstring[searchstringlen-5]='?';
	searchstring[searchstringlen-4]='.';
	searchstring[searchstringlen-3]='*';
	searchstring[searchstringlen-2]='$';
	searchstring[searchstringlen-1]='\0';
	
	
	if(regcomp(&preg,searchstring,REG_EXTENDED)!=0)
	{
		perror("setpos_at_macro:regcomp:");
		regfree(&preg);
		free(pmatch);
		free(searchstring);

		return -1;
	}
	
	rewind(node->env->file);
	lastfp=ftell(node->env->file);

	while( (returnval=getline(&line,&linelen,node->env->file)) != -1 )
	{
		reerror = regexec(&preg, line, nmatch, pmatch, 0);
		if (reerror == 0)
		{
		/* located matching line for macro */

			/* is this line a macro label? (ending in ':') */
			reerror = regexec(node->env->lex->macro_label_preg, line, nmatch, pmatch, 0);
			if (reerror == 0)
			{
			/* if yes, keep the file pointer where it is */
				break;
			}else{
			/* if no, reset the file pointer to the beginning of this line */
				fseek(node->env->file,lastfp,SEEK_SET);
				break;
			}
		}
		lastfp=ftell(node->env->file);
	}
	free(line);
	regfree(&preg);
	free(pmatch);
	free(searchstring);

	return returnval;
}


int setpos_first_rooted_block(Node *node)
{
	/* read root->env->file until first valid sequence  */
	/* valid sequence:  "^[^[:space:]]" */

	static int 	lastc	= '\0';
	int 		thisc	= '\0';
	int		nextc	= '\0';
	char		*line	= NULL;
	size_t		linelen = 0;


	if( (thisc=fgetc(node->env->file)) != EOF )
	{
		switch(thisc)
		{
			case '#':
				if( nextc=fgetc(node->env->file) == '!')
				{
					if(getline(&line,&linelen,node->env->file) == -1)
					{
						// todo: handle error
					}
				}else{
					ungetc(nextc,node->env->file);
					ungetc(thisc,node->env->file);
				}
				break;
			case ' ':
			case '	':
			case '\n':
				lastc=thisc;
				setpos_first_rooted_block(node);
				break;
			default:
				if(lastc=='\n')
				{
					ungetc(thisc,node->env->file);
				}else{
					if(lastc=='\0')
					{
						ungetc(thisc,node->env->file);
					}else{
						setpos_first_rooted_block(node);
					}
				}
		}
	}else{
		return EOF;
	}
	return 0;
}

int open_file(Node *node)
{
	/*----- todo: improve return values -----*/

	/*----- TEST IF FILENAME NOT NULL -----*/
	if( node->env->filepath == NULL )
	{
		return -1;
	}

	/*----- TEST IF FILE EXISTS -----*/
	struct stat statbuffer;
	if( stat(node->env->filepath, &statbuffer) == -1 )
	{
		perror(NULL);
		return -1;
	}
	
	/*----- OPEN FILE -----*/
	if((node->env->file=fopen(node->env->filepath,"r"))==NULL)
	{
		return -1;
	}

	/*----- TODO: add file content check here  -----*/
	/*	weed out innappropriate formats.	*/
	/*	right now this program is only designed */
	/*	to parse plain text whitespace indented */
	/*	outlines.				*/

	/*----- SET FILE POSITION TO FIRST ROOTED BLOCK -----*/
	if(setpos_first_rooted_block(node)==EOF)
	{
		/* if we can't find a rooted block - 	*/
		/* ie. (^[^[:blank:]]+.*)		*/
		/* then there's nothing to parse	*/

		return -1;
	}

	return 0;
}


/*-------- PARSE FUNCTIONS -------------*/

void get_node_rawline(Node *node)
{
	char	*line 	= NULL;
	char	*newline= "\n";
	size_t	linelen	= 0;

	if((getline(&line,&linelen,node->env->file)) != -1)
	{
		node->rawline=line;
	}else{
		free(line);
		node->rawline=strdup(newline);
	}
}

int  get_indent(char *inputstring)
{
	int i=0;
	int indent=0;

	if(inputstring!=NULL)
	{
		for(i=0; i < strlen(inputstring)+1; i++)
		{
			if(inputstring[i]==' ') indent++;
			if(inputstring[i]=='	') indent+=TABWIDTH;
			if(inputstring[i]!=' ' && inputstring[i]!='	') break;
		}
	}
	return indent;
}


char *string_from_regmatch(char *src, regmatch_t *pmatch, int substrno)
{
	char *dest;
	int copylen = pmatch[substrno].rm_eo - pmatch[substrno].rm_so;
	if( (dest=strndup( src+pmatch[substrno].rm_so, (copylen + 1))) != NULL)
	{
		dest[copylen] = '\0';
	}else{
		perror("substring from pmatch, strndup out of memory");
		return NULL;
	}
	return dest;	
}

/*------------ PRIMARY PARSE FUNCTION: PARSES LINE INTO NODE STRUCT --------------*/

void parse_node(Node *node)
{	
	/*--- VARS FOR REGEX MATCHES ---*/
	size_t nmatch		= 10;
	regmatch_t *pmatch	= malloc(sizeof(regmatch_t)*nmatch);
	regmatch_t *pmatch_2	= malloc(sizeof(regmatch_t)*nmatch);

	/*--- NODE: rawline ---*/
	get_node_rawline(node);

	/*	BLANKLINE?	*/
	if(regexec(node->env->lex->blankline_preg, node->rawline, nmatch, pmatch, 0)==0)
	{
		/*--- NODE: type ---*/
		node->type=DIVINE_BLANKLINE;
		/*--- Force blank lines to newline ---*/
		node->rawline[0]='\n';
		node->rawline[1]='\0';

		free(pmatch);
		free(pmatch_2);
		return;
	}

	/*	HTML_NOCLOSE?	MUST PRECEDE AND EXCLUDE 'MARKUP' TEST	*/
	if(regexec(node->env->lex->html_noclose_preg, node->rawline, nmatch, pmatch_2, 0)==0)
	{
		node->type=DIVINE_HTML_NOCLOSE;
		node->opentag=string_from_regmatch( node->rawline,
						    pmatch_2,
						    DIVINE_HTML_NOCLOSE_MATCH);
		if(node->opentag==NULL)
		{ printf("string_from_regmatch returned NULL\n");}
	}else{
		/*	MARKUP?		*/
		if(regexec(node->env->lex->markup_preg, node->rawline, nmatch, pmatch, 0)==0)
		{
			/*--- NODE: type ---*/

			node->type=DIVINE_MARKUP;

			/*--- NODE: htmlelement	---*/

			node->htmlelement=string_from_regmatch( node->rawline,
								pmatch,
								DIVINE_MARKUP_ELEMENT);
			if(node->htmlelement==NULL)
			{ /* todo: error handler here */ }


			/*	IDENT?		*/
			if(regexec(node->env->lex->ident_preg, node->rawline, nmatch, pmatch_2, 0)==0)
			{
				/*	NODE: tagid	*/
				node->tagid=string_from_regmatch( node->rawline,
								  pmatch_2,
								  DIVINE_IDENT_MATCH);
				if(node->tagid==NULL)
				{ /* todo: error handler here */ }
			}
			/* tagclass	IGNORED		*/
			/* opentag			*/
			node->opentag=string_from_regmatch( node->rawline,
							    pmatch,
							    DIVINE_MARKUP_OPENTAG );
			if(node->opentag==NULL)
			{ /* todo: error handler here */ }
			
			/* closetag	*/
			if(node->type!=DIVINE_HTML_NOCLOSE)
			{
				/* otherwise leave node->closetag=NULL	*/	
			}
			
		}else{
		/*	IDENT?		*/
		/*	CLASS?		*/
			if( regexec( node->env->lex->ident_preg2, node->rawline, nmatch, pmatch, 0)==0)
			{
			/* htmlelement */
				char *divtype="div";
				node->htmlelement=strdup(divtype);
			/* tagid */
				if( pmatch[DIVINE_IDENT2_TAGID].rm_so != -1)
				{
					node->type=DIVINE_IDENT;
					node->tagid=string_from_regmatch( node->rawline,
								       pmatch,
								       DIVINE_IDENT2_TAGID );
					if(node->tagid==NULL)
					{	/* empty errorcheck */		}
				}
			/* tagclass */
				if( pmatch[DIVINE_IDENT2_CLASS].rm_so != -1)
				{
					if(node->type!=DIVINE_IDENT)
					node->type=DIVINE_CLASS;
					node->tagclass=string_from_regmatch( node->rawline,
								 	  pmatch,
									  DIVINE_IDENT2_CLASS );
					if(node->tagclass==NULL)
					{	/* empty errorcheck */		}
				}
			/* opentag */
				/*	output opentag on the fly later in parse_tree()	*/
			/* closetag */
				/*	output closetag on the fly later in parse_tree() */
			}
			
			/*	TEXT?		*/
			/* WARNING: IDENT and CLASS regex also match TEXT */
			/* we have to explicitly change the node->htmlelement */

			if(regexec(node->env->lex->text_preg,
				   node->rawline, nmatch, pmatch, 0)==0)
			{
				node->type=DIVINE_TEXT;
				node->opentag=string_from_regmatch( node->rawline,
								    pmatch,
								    DIVINE_TEXT_MATCH );
				if(node->opentag==NULL);
				{	/* empty errorcheck */		}

				/* change the htmlelement here if IDENT logic has set it */
				if(node->htmlelement!=NULL)
				{
					free(node->htmlelement);
					node->htmlelement=NULL;
				}
			}
			/*	MACRO?		*/
			if(regexec(node->env->lex->macro_preg,
				   node->rawline, nmatch, pmatch, 0)==0)
			{
				node->type=DIVINE_MACRO;
				node->opentag=string_from_regmatch( node->rawline,
								    pmatch,
								    DIVINE_MACRO_MATCH );
				if(node->opentag!=NULL)
				{	/* empty errorcheck */		}
			}
		}
	}
	/*	NODE: rawindent		*/
	/*   todo: if not blankline check */
	node->rawindent=get_indent(node->rawline);

	/* after everything */
	free(pmatch);
	free(pmatch_2);
}


/*-------------------- MAIN PARSE LOOP: PARSES OUTLINE TREE --------------------------*/

Node *parse_tree(Node *node)
{
	int	closed		=  1;
	long	restorefpi	= -1;
	Node 	*nextnode	= new_node(node->env);
	Node 	*nextnextnode	= NULL;

	/*--- parse_tree: ---*/
	/* . prints opening and closing tags for 'node'			*/
	/*	(waits to print closing tag after all recursions)	*/
	/* . calls 'parse_node(nextnode)' to parse next line of input	*/
	/* . recurses as needed for child and sibling lines 		*/
	/* . returns a link to 'nextnode' from recursion		*/
	/* . returns a link to NULL when finished.			*/
	/*	(when 'nextnode->rawline[0]' is newline)			*/
	

	/*----------- MACRO EXPANSION - SETS FPI  ---------*/
	/*   If type is a macro, then we need to 			*/
	/*   save the current file position indicator,	 'restorefpi'	*/
	/*   search for a macro expansion in the file,			*/
	/*   set the file position indicator,				*/
	/*   and continue parsing until a blank line.			*/


	if(node->type==DIVINE_MACRO)
	{
		restorefpi = ftell(node->env->file);

		if( (node->macrostat=setpos_at_macro(node)) != -1 )
		{

			parse_node(nextnode);
			output_opentag(nextnode);

			if(node->htmlelement    !=NULL &&
			   nextnode->htmlelement!=NULL   )
			{
				free(node->htmlelement);
				node->htmlelement	= strdup(nextnode->htmlelement);
			}
			
			if(node->tagid	   !=NULL &&
			   nextnode->tagid !=NULL   )
			{
				printf("JACKINTHEBOX! node->tagid %d\n",node->tagid);
				printf("JACKINTHEBOX! nextnode->tagid %d\n",node->tagid);
				free(node->tagid);
				node->tagid	= strdup(nextnode->tagid);
			}

			node->type=nextnode->type;

			nextnode->macroflag=1;
			Node *errnode=parse_tree(nextnode);
			// output_closetag(node);
			

			/*--- parse_tree shouldn't return a node ---*/
			/*    but just in case we will handle it    */
			if(errnode!=NULL)
			{
				printf("errnode is not NULL\n");
				rm_node(errnode);
				errnode=NULL;
			}
			

			rm_node(nextnode);
			nextnode=new_node(node->env);
		}
		fseek(node->env->file, restorefpi, SEEK_SET);

	}else{
		output_opentag(node);
	}


	/*--------- PARSE NEXT LINE - USES FPI --------*/

	/*   This call reads the next line from the file		*/
	/*   and parses it into a 'Node *nextnode'			*/

	parse_node(nextnode);

	
	/*--------- RETURN FROM MACRO EXPANSION - SETS FPI --------*/
	/*   If the 'nextnode' is a blank line or newline		*/
	/*   check to see if it's the end of a macro			*/
	/*   ( for a macro, 'restorefpi' will not be -1)		*/
	/*   If this was the end of a macro expansion we have to restore */
	/*   the file position to where we left off and we need to read  */
	/*   the next line in the file into 'nextnode'.			*/


	/*   Now, if the next line is still blank or newline,		*/
	/*   it's the end of the primary outline.			*/
	/*   skip any more recursions 					*/

	if(nextnode->rawline[0]!='\n')
	{
		/*   But if the nextnode isn't blank or newline:		*/
		/*   We have to decide whether to print a close tag		*/
		/*   (in the case of a sibling) or recurse into a child		*/
		/*   or recurse into a macro					*/
		/*   and then print the close tag after recursion is finished.	*/

		/*------------- RECURSE NEXT NODE -------*/
		do
		{
			/*   We will enter a simple 'do' loop and evaluate 		*/
			/*   for a new newline or blankline at the end of the loop.	*/
				
			
			/*-------- SIBLING --------*/
			
			/*   In the simplest circumstance, perhaps, there will be no	*/
			/*   change in indent level from one line of input to the next. */
			/*   We can just print the close tag.  And then move to the	*/
			/*   next line.							*/

			if(nextnode->rawindent == node->rawindent)
			{
				/*   The 'nextnode->indent' object variable		*/
				/*   is used for adjusting the				*/
				/*   'node->env->outputindent' for legible output.	*/

				nextnode->indent=node->indent;

				if(closed==1)
				{
					output_closetag(node);
					closed=0;
				}
				/*   Now that we've printed the current close tag	*/
				/*   we need to move on to the 'nextnode'.		*/
				/*   This will actually create a recursion and could	*/
				/*   potentially cause two close tags to print...	*/

				nextnextnode=parse_tree(nextnode);

				/*   After passing 'nextnode' to the recursion and	*/
				/*   returning the 'nextnextnode' we can free		*/
				/*   'nextnode' and set 'nextnode' to 'nextnextnode'	*/

				// printf("free-a\n");
				rm_node(nextnode);
				nextnode=nextnextnode;

				/*   If the new 'nextnode->rawline' is blank or newline  */
				/*   The end 'while' of the 'do' loop will catch it	*/
				/*   and end the loop.					*/
				/*   Otherwise, it could be that the next line from	*/
				/*   the file is some number of indents less than the   */
				/*   current line.  We'll test for that in a second.    */
				continue;
			}
		

			/*-------- NEXT LINE HAS LESS INDENTS ------*/
			if(nextnode->rawindent < node->rawindent)
			{

				if(closed==1)
				{
					output_closetag(node);
					closed=0;
				}
				
				nextnode->indent--;

				return nextnode;
			}

			/*-------- CHILD ------*/
			/*   If the indent level of the next line in the file		*/
			/*   is greater than the current line, then we need to wait	*/
			/*   to print the closing tag of the current node.		*/
			/*   And we need to adjust the 'node->env->outputindent'.	*/
			/*   To print the child's tags we recurse.			*/

			if(nextnode->rawindent > node->rawindent)
			{
				nextnode->indent++;
				node->env->outputindent++;

				nextnextnode=parse_tree(nextnode);
				// printf("afterchild:\n");

				// printf("free-b\n");
				rm_node(nextnode);
				nextnode=nextnextnode;
				
				node->env->outputindent--;

				continue;
			}

			

		} while(nextnode!=NULL);


		if(closed==1)
		{
			output_closetag(node);
			closed=0;
		}
		if(nextnode!=NULL)
		{
			rm_node(nextnode);
			nextnode=NULL;
		}
	}else{

		/*   If you're here, the last line read from the file		*/
		/*   was a blank line or a newline.				*/
		/*   Recursion was skipped.					*/
		
		/*   This was either a blank line or newline at the end		*/
		/*   of the primary outline in the file,			*/

		/*   The possibility of this being the blank line at 		*/
		/*   the end of a macro expansion, has already been checked	*/
		/*   near the beginning of this function, before the loop.	*/

		/*   The 'nextnode' object is finished and must be free'd.	*/
		/*   And we must print the closetag for current 'node'		*/
		/*   if the node has not been 'closed'.				*/
		
		if(closed==1)
		{
			output_closetag(node);
			closed=0;
		}
		rm_node(nextnode);
		nextnode=NULL;
	}


	/*--------- RETURN VAL -----------*/
	return nextnode;
}




/*------------------- MAIN ----------------------*/
/* usage: divinity <file>	*/

int  main (int argc, char **argv)
{
	/*----- TEST FOR CORRECT NUMBER OF ARGS -----*/
	if(argc!=2)
	{
		printf("Usage: %s <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*----- SETUP MAIN DATA STRUCTURE -----*/
	Env	*newenv	 =	new_env();
	Node 	*root	 =	new_node(newenv);

	
	/*----- SET INPUT -----*/
	root->env->filepath   =	argv[1];



	/*----- OPEN FILE -----*/
	if ( open_file(root) == -1 )
	{
		exit(EXIT_FAILURE);
	}

	/*----- INIT LEXER -----*/
	init_lex(&newenv->lex);


	
	/*----- TEST FOR CGI CONTEXT, OUTPUT HTTP HEADER -----*/
	/* todo: is GATEWAY_INTERFACE ubiquitous?	      */
	/*	 if not, use a different env var to test      */
	/*	 for CGI context.			      */
	/*	 Also, what if this program is piped?	      */
	/*	 There could be another circumstance where    */
	/*	 merely checking an env var could cause a     */
	/*	 unwanted header midstream.  		      */

	char *evar;
	#ifdef _GNU_SOURCE
	if ( (evar=__secure_getenv("GATEWAY_INTERFACE")) != NULL )
	#else
	if ( (evar=getenv("GATEWAY_INTERFACE")) != NULL )
	#endif
	{
		printf("Content-Type: text/html; charset=UTF-8\n");
		printf("Content-Language: en\n");
		printf("\n");
	}


	/*----- MAIN PROCEEDURE -----*/
	
	parse_node(root);

	Node *errnode=parse_tree(root);
	if(errnode!=NULL)
	{
		printf("main:errnode: there's an extra node returned\n","");
		printf("main:errnode: ...we must destroy it...\n","");
		rm_node(errnode);
	}

	fclose(root->env->file);
	rm_node(root);

	return 0;
}
