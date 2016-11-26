/*  This file grammer follows 'lex' conventions. */
/*  Defines rules for objects within a single line of input. */
%%

^[[:space:]]+$				{	node->type = DIVINE_EMPTY;	}
^[<]([[:alnum:]_]+[[:blank:]]*)+[>].*$	{	node->type = DIVINE_MARKUP;	}
^[[:blank:]]*([[:alnum:]_]+[[:blank:]]*)+$



%%
