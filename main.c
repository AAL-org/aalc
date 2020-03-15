/*                                                                      
*                          ````..........````                          
*                     ``...-------------------..``                     
*                 ``..----------------------------..``                 
*               `.------------------------------------.`               
*            `.-----------------------------------------..`            
*          `..--------------------------------------------..`          
*         .---------------------SEND HELP-------------------..         
*       `.----------------------------------------------------.`       
*      `.------------------------------------------------------.`      
*     `.--------------------------------------------------------.`     
*    `.----------------------------------------------------------.`    
*    .-------------/yso/-----------------------/oshy:-------------.    
*   .------------..:oshddy+-----------------:shid.+/...------------.   
*   .---------..`      ./hdh+-------------:yddo-       `..---------.   
*  `---------.`    `.`    :hdy-----------/dds.   `--.    `.---------`  
*  `--------.`   -ydddy-   .hdh---------:ddo    +daddy.   `---------`  
*  .--------.    yydaddy    -+/----------//.   .dddddd+   `.--------.  
*  `--------.`   -hdddh:   `.--------------.    /hdddo`   .---------`  
*  `--------:/`    .-.    `.----------------.`    ``     -/:--------`  
*   .-----:////:.      `-:::----------------:::-`     `-/////------.   
*   .----/////////::::ydddddddddhhhhhhhhdddddddddy:://////////:----.   
*    ..-////////////////+oosyyyhhhhhhhhhhyyysso+////////////////-..    
*    `:///////////////:                          ////////////////:`    
*    ./////////////////hs/.                  ./sy/////////////////.    
*    ://///////////////hddddyso//:::::://osyddddh/////////////////:    
*    -/////////////////dddddddddddddddddddddddddh/////////////////-    
*    `:///////////////oddddddddddddddheckddddddddo///////////////:`    
*     `-/////////////--:ohddddddddddddddddddddho:--/////////////-`     
*       `-:///////:--------/oyhddddddddddhyo/--------:///////:-`       
*           `````..--------------::::::---------------.`````           
*                 ``..----------------------------..``                 
*                     ``...------------------...``                     
*                          ````..........````                          
*/                                                                      

/* CURRENT STATE OF BEING
* so when ever it back tracks it doesn't put the old token back
* i am too lazy to write something to go back properly
* i could use the old token or an array of token, but nah
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define mkleaf(x,y) mknode(x,y,NULL,NULL)
#define mkunary(x,y,z) mknode(x,y,z,NULL)

#define semi() match(T_END)
#define ident() match(T_IDNT)

#define openbracket() match(T_BKO)
#define closebracket() match(T_BKC)
#define openparentheses() match(T_PRO)
#define closeparentheses() match(T_PRC)

#define insertsymbolvalue(type,t) {switch (type){ \
		case INT:{symbol_v.integer = t.integer; break;} \
		case FLOAT:{symbol_v.decimal = t.decimal; break;} \
		case STRING:{symbol_v.string = t.string; break;}} \
		}

void fatal (const char* message, ...)
{
	va_list args;
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	exit(1);
}

enum TOKEN_TYPE
{
	T_ADD = 0, T_SUB, T_MUL, T_DIV, T_ASN, T_SEP, // operators
	T_FN, T_INT, T_FLT, T_STR, // keywords
	T_INTLIT, T_FLTLIT, T_STRLIT, // literals
	T_BKO, T_BKC, T_PRO, T_PRC, T_END, // syntax
	T_IDNT,
};

static const char *TOKEN_TYPE_DEBUG[] = 
{
	"+", "-", "*", "/", ":", ",", // operators
	"fn", "int", "float", "str", // keywords
	"INTEGER", "FLOAT", "STRING", // literals
	"{", "}", "(", ")", ";", // syntax
	"IDENTIFIER"
};

enum AST_OP
{
	A_ASN, A_STAT, A_FUNC, A_PRNT, A_EXPR, A_COMB
};

enum SYMBOL_TYPE
{
	INT = 0,
	FLOAT,
	STRING,
	FUNCTION,
};

typedef struct token
{
	int token;
	union {
		long int integer;
		double decimal;
		char *string;
	};
} token;

typedef struct astnode
{
	int op;
	int value;
	struct astnode *left;
	struct astnode *right;
} astnode;

typedef struct symbol 
{
	int type;
	const char* k;
	union {
		long int integer;
		double decimal;
		char* string;
	} v;
} symbol;

#define TABLESIZE 1024
// global trash
FILE* infile;
int c, putbc = 0, line = 0, tbnxt = 0;
token T, oT;
symbol table[TABLESIZE];
union {long int integer;double decimal; char* string;} symbol_v;

static int insertsymbol (int type, const char *key)
{
	if (tbnxt == 1024)
		fatal("Symbol table has exceded 1024 items");
	table[tbnxt].type = type;
	table[tbnxt].k = key;
	switch (type)
	{
		case INT:
			table[tbnxt].v.integer = symbol_v.integer;
			break;
		case FLOAT:
			table[tbnxt].v.decimal = symbol_v.decimal;
			break;
		case STRING:
			table[tbnxt].v.string = symbol_v.string;
			break;
	}
	return tbnxt++;
}

static int findsymbol (const char* key)
{
	for (int i = 0; i < tbnxt; ++i)
	{
		if (!strcmp(key, table[i].k))
			return i;
	}
	return -1;
}

static void putback(int c_) { putbc = c_; }

static char* stringscan(void)
{
	int i = 0;
	char* s = (char*)malloc(256);
	if (s == NULL)
		fatal("failed malloc");
	s[i] = fgetc(infile);
	while (s[i] != '"')
		s[++i] = fgetc(infile);
	s[i] = 0;
	
	return s;
}

static void identscan(void)
{
	int i = 0; char* s = (char*)malloc(256);
	if (s == NULL)
		fatal("failed malloc");
	s[0] = c;
	while ((s[i] <= 'z' && s[i] >= 'a') || (s[i] <= 'Z' && s[i] >= 'A') || (s[i] <= '9' && s[i] >= '0') || s[i] == '_')
		s[++i] = fgetc(infile);
	// put back the invalid char and end the string
	putback(s[i]);
	s[i] = 0;

	// Set the token type
	T.token = T_IDNT;
	T.string = s;
	switch (s[0])
	{
		case 'f':{ // fn, float
			if(!strcmp("float", s))
				T.token = T_FLT;
			else if (!strcmp("fn", s))
				T.token = T_FN;

			break;}
		case 'i':{ // int
			if(!strcmp("int", s))
				T.token = T_INT;

			break;}
		case 's':{ // str
			if(!strcmp("str", s))
				T.token = T_STR;

			break;}
	}
}

// wtf lmao, i'll clean this later
static void numscan (void)
{
	long int val = 0, k, dodec = 0;
	double d, decc = 1;
	char* p;
	while ((p = strchr((char*)"0123456789.", c)) != NULL)
	{
		if (*p == '.')
		{
			if (dodec)
				fatal("you cant have two decimals! line: %d", line);
			dodec = 1;
			d = val;
			c = fgetc(infile);
			continue;
		}
		k = (*p) - '0';
		if (dodec){
			float t = 0.1;
			for (int i = 1; i < decc; ++i)
				t *= 0.1;
			d = d + t * (float)k;
			++decc;
		}
		else
			val = val * 10 + k;
		c = fgetc(infile);
	}
	putback(c);
	T.token = dodec ? T_FLTLIT : T_INTLIT;
	dodec ? (T.decimal = d) : (T.integer = val);
}

static int whitespace (int c)
{
	if (c == ' ' || c == '\t')
		return 1;
	else if (c == '\n')
		++line;
	else
		return 0;
	return 1;
}

static int next (void)
{
	// get the current next character
	if (putbc) {
		c = putbc;
		putbc = 0;
	} else
		c = fgetc(infile);

	// check its not EOF
	if (c == EOF)
		return 0;

	// skip whitespace
	if (whitespace(c))
		return next();

	switch (c)
	{
		case '+':
			T.token = T_ADD;
			break;
		case '-':
			T.token = T_SUB;
			break;
		case '*':
			T.token = T_MUL;
			break;
		case '/':
			{
				c = fgetc(infile);
				if (c == '?')
				{
					while (c != '\n')
					{c = fgetc(infile); if (c == EOF) return 0;}
					++line;
					return next();
				}
				putback(c);
				T.token = T_DIV;
				break;
			}
		case '"':
			T.token = T_STRLIT;
			T.string = stringscan();
			break;
		case '(':
			T.token = T_PRO;
			break;
		case ')':
			T.token = T_PRC;
			break;
		case '{':
			T.token = T_BKO;
			break;
		case '}':
			T.token = T_BKO;
			break;
		case ':':
			T.token = T_ASN;
			break;
		case ';':
			T.token = T_END;
			break;
		default:
		{
			if (isdigit(c))
				numscan();
			else if (isalpha(c))
				identscan();
			else
				fatal("Unknown character %c at line %d\n", c, line);
			break;
		}
	}
	return 1;
}

static void match (int op)
{
	next();
	if (T.token == op)
		oT = T;
	else
		fatal("Expected: %s at line: %d", TOKEN_TYPE_DEBUG[op], line);
}

astnode* mknode (int op, int value, astnode *left, astnode *right)
{
	astnode *n = (astnode*)malloc(sizeof(astnode));
	if(!n)
		fatal("Couldn't malloc node");

	n->op = op;
	n->value = value;
	n->left = left;
	n->right = right;
	return n;
}

astnode* parse_expression()
{
	match(T_STRLIT);
	insertsymbolvalue(STRING, T);
	int index = insertsymbol(STRING, "anonmous");

	return mkleaf(A_EXPR, index);
}

astnode* parse_declaration()
{
	next(); // THESE TOO
	// If it doesn't look like a declaration backtrack
	if (!(T.token == T_INT || T.token == T_FLT || T.token == T_STR))
		return NULL;
	int type = T.token-T_INT;

	// Match a identifier
	ident();
	char* name = T.string;

	// Assign symbol
	match(T_ASN);
	
	// Match the value
	match(type+T_INTLIT);

	// add it to the symbol table
	insertsymbolvalue(type, T);
	int index = insertsymbol(type, name);

	return mkleaf(A_ASN, index);
}

astnode* parse_print_statement()
{
	next(); // THESE 3 LINES ARE THE PROBLEM
	if (T.token != T_IDNT)
		return NULL;
	openparentheses();

	astnode *left = parse_expression();

	closeparentheses();

	return mkunary(A_PRNT, -1, left);
}

astnode* parse_statment()
{
	astnode *left;
	// try to parse a print statement
	left = parse_print_statement();
	if (left != NULL){
		printf("parsed print statment\n");
		return mkunary(A_STAT, -1, left);
	}
	// try to parse a declaration
	left = parse_declaration();
	if (left != NULL){
		printf("parsed declaration statment\n");
		return mkunary(A_STAT, -1, left);
	}

	return NULL;
}

astnode* parse_compound_statement()
{
	astnode *left = NULL, *tree;
	openbracket();

	for(;;)
	{
		tree = parse_statment();
		semi();

		if (tree != NULL)
		{
			if (left == NULL)
				left = tree;
			else
				left = mknode(A_COMB, -1, left, tree);
		}
		next();
		
		if (T.token == T_BKC)
		{
			closebracket();
			return left;
		}
	}
}

astnode* parse_function()
{
	astnode *left;

	// Match function signifier
	match(T_FN); 

	// function name, idk how to do functions
	ident();
	int i = insertsymbol(FUNCTION, T.string);
	// parameters
	openparentheses(); closeparentheses();
	// function body
	left = mkunary(A_FUNC, i, parse_compound_statement());

	return left;
}

astnode* mkast (void)
{
	return parse_function();
}

int main (int argc, char *argv[])
{
	if (argc < 2)
		fatal("cannot compile without a input file\nUSAGE: %s <input>\n", argv[0]);
	infile = fopen(argv[1], "r");
	while(next())
	{
		if ((T.token >= T_ADD  && T.token <= T_FLT) || (T.token >= T_BKO  && T.token <= T_END))
			printf("token: \'%s\'\n", TOKEN_TYPE_DEBUG[T.token]);
		else if (T.token == T_INTLIT)
			printf("token: %s\t\t value: %ld\n", TOKEN_TYPE_DEBUG[T.token], T.integer);
		else if (T.token == T_FLTLIT)
			printf("token: %s\t\t value: %lf\n", TOKEN_TYPE_DEBUG[T.token], T.decimal);
		else if (T.token == T_IDNT)
			printf("token: %s\t value: %s\n", TOKEN_TYPE_DEBUG[T.token], T.string);
		else if (T.token == T_STRLIT)
			printf("token: %s\t\t value: \"%s\"\n", TOKEN_TYPE_DEBUG[T.token], T.string);
	}
	// Reset the global things
	fseek(infile, 0, SEEK_SET);
	c = 0; line = 0;

	astnode *root = mkast();

	fclose(infile);
	exit(0);
}