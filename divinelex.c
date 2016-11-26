
/* ----------- LEXICAL TYPES -------------------*/

/* the following macros are assigned to node->type by init_node() */
/* each of these is a type of line from the input file.		*/

#define DIVINE_MARKUP		'm'
#define DIVINE_HTML_NOCLOSE	'N'
#define DIVINE_IDENT		'i'
#define DIVINE_CLASS		'c'
#define DIVINE_TEXT		't'
#define DIVINE_MACRO		'o'
#define DIVINE_BLANK		'b'
#define	DIVINE_BLANKLINE 	'e'


/* ----------- REGEX ----------- */

#define DIVINE_BLANKLINE_RE	"^[[:space:]]*$"
#define DIVINE_MARKUP_RE 	"^[[:blank:]]*(<([_[:alnum:]]+)(.*))\n$"
								#define DIVINE_MARKUP_MATCH	0
								#define DIVINE_MARKUP_OPENTAG	1
								#define DIVINE_MARKUP_ELEMENT 	2
#define DIVINE_HTML_NOCLOSE_RE "^[[:blank:]]*(<([!]{1}|area|base|br|col|command|embed|hr|img|input|keygen|link|meta|param|source|track|wbr).*?>)"
								#define DIVINE_HTML_NOCLOSE_MATCH 0
#define DIVINE_IDENT_RE		"id=[\"]?([[:alpha:]][-_.[:alnum:]]*)[\"]?"
									#define DIVINE_IDENT_MATCH 1
#define DIVINE_IDENT_RE2	"^[[:blank:]]*([[:alpha:]][-_[:alnum:]]*)?[[:blank:]]*([.]([[:alpha:]][-_[:alnum:]]*))?"
									#define DIVINE_IDENT2_TAGID 1
									#define DIVINE_IDENT2_CLASS 3
/* #define DIVINE_TEXT_RE		"^[[:blank:]]*[\"](|.*[^\"])[\"]?.*[\n]" */
#define DIVINE_TEXT_RE		"^[[:blank:]]*([\"]|[\'])(|.*)[\n]$"
									#define DIVINE_TEXT_MATCH   2

#define DIVINE_MACRO_RE		"^[[:blank:]]*[$]([-_.[:alnum:]]+).*\n$"
									#define DIVINE_MACRO_MATCH  1

#define DIVINE_MACRO_LABEL_RE	"^[-_.[:alnum:]]+[:]"
								#define DIVINE_MACRO_LABEL_MATCH 0

